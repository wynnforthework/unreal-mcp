"""
Enhanced UE Editor Python Execution Tool

Implements a local RPC/command line tool that provides enhanced Python execution capabilities 
in UE Editor. Extends the existing UMG MCP tools with batch operations, transaction support, 
and better error handling.
"""

import asyncio
import json
import socket
import time
import uuid
from typing import Dict, Any, List, Optional, Callable, Union, Tuple
from dataclasses import dataclass, field
from enum import Enum
import logging


class TransactionStatus(Enum):
    """Status of a transaction"""
    PENDING = "pending"
    RUNNING = "running"
    COMMITTED = "committed"
    ROLLED_BACK = "rolled_back"
    FAILED = "failed"


@dataclass
class UECommand:
    """Represents a single UE command"""
    id: str
    command_type: str
    params: Dict[str, Any]
    timeout: float = 30.0
    retry_count: int = 0
    max_retries: int = 3
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "type": self.command_type,
            "params": self.params
        }


@dataclass
class CommandResult:
    """Result of executing a UE command"""
    command_id: str
    success: bool
    data: Optional[Dict[str, Any]] = None
    error: Optional[str] = None
    execution_time: float = 0.0
    retry_count: int = 0


@dataclass
class Transaction:
    """Represents a transaction containing multiple commands"""
    id: str
    commands: List[UECommand] = field(default_factory=list)
    status: TransactionStatus = TransactionStatus.PENDING
    results: List[CommandResult] = field(default_factory=list)
    rollback_commands: List[UECommand] = field(default_factory=list)
    created_at: float = field(default_factory=time.time)
    
    def add_command(self, command: UECommand, rollback_command: Optional[UECommand] = None):
        """Add a command to the transaction"""
        self.commands.append(command)
        if rollback_command:
            self.rollback_commands.append(rollback_command)


class UEPythonTool:
    """
    Enhanced Python execution tool for UE Editor.
    
    Features:
    - Batch command execution
    - Transaction support with rollback
    - Connection pooling and retry logic
    - Performance monitoring
    - Command queuing and prioritization
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        self.config = config or {}
        self.host = self.config.get("host", "127.0.0.1")
        self.port = self.config.get("port", 55557)
        self.max_connections = self.config.get("max_connections", 5)
        self.connection_timeout = self.config.get("connection_timeout", 10.0)
        self.command_timeout = self.config.get("command_timeout", 30.0)
        
        # Connection pool
        self.connection_pool: List[Tuple[asyncio.StreamReader, asyncio.StreamWriter]] = []
        self.pool_lock = asyncio.Lock()
        
        # Transaction management
        self.transactions: Dict[str, Transaction] = {}
        self.active_transaction: Optional[str] = None
        
        # Command queue
        self.command_queue: asyncio.Queue = asyncio.Queue()
        self.queue_processor_task: Optional[asyncio.Task] = None
        
        # Performance tracking
        self.performance_stats = {
            "commands_executed": 0,
            "total_execution_time": 0.0,
            "failed_commands": 0,
            "average_response_time": 0.0
        }
        
        self.logger = logging.getLogger("ue_python_tool")
    
    async def start(self):
        """Start the tool and initialize connection pool"""
        self.logger.info("Starting UE Python Tool")
        
        # Initialize connection pool
        await self._initialize_connection_pool()
        
        # Start command queue processor
        self.queue_processor_task = asyncio.create_task(self._process_command_queue())
        
        self.logger.info("UE Python Tool started")
    
    async def stop(self):
        """Stop the tool and cleanup resources"""
        self.logger.info("Stopping UE Python Tool")
        
        # Stop queue processor
        if self.queue_processor_task:
            self.queue_processor_task.cancel()
            try:
                await self.queue_processor_task
            except asyncio.CancelledError:
                pass
        
        # Close all connections
        await self._close_all_connections()
        
        self.logger.info("UE Python Tool stopped")
    
    async def execute_command(self, command_type: str, params: Dict[str, Any],
                            timeout: Optional[float] = None) -> CommandResult:
        """Execute a single command"""
        
        command = UECommand(
            id=str(uuid.uuid4()),
            command_type=command_type,
            params=params,
            timeout=timeout or self.command_timeout
        )
        
        return await self._execute_single_command(command)
    
    async def execute_batch(self, commands: List[Dict[str, Any]],
                          parallel: bool = False) -> List[CommandResult]:
        """Execute multiple commands in batch"""
        
        ue_commands = []
        for i, cmd_data in enumerate(commands):
            command = UECommand(
                id=f"batch_{uuid.uuid4()}_{i}",
                command_type=cmd_data["command_type"],
                params=cmd_data["params"],
                timeout=cmd_data.get("timeout", self.command_timeout)
            )
            ue_commands.append(command)
        
        if parallel:
            return await self._execute_commands_parallel(ue_commands)
        else:
            return await self._execute_commands_sequential(ue_commands)
    
    async def begin_transaction(self) -> str:
        """Begin a new transaction"""
        
        if self.active_transaction:
            raise RuntimeError("Another transaction is already active")
        
        transaction_id = str(uuid.uuid4())
        transaction = Transaction(id=transaction_id)
        
        self.transactions[transaction_id] = transaction
        self.active_transaction = transaction_id
        
        self.logger.info(f"Started transaction {transaction_id}")
        return transaction_id
    
    def add_to_transaction(self, command_type: str, params: Dict[str, Any],
                          rollback_command_type: Optional[str] = None,
                          rollback_params: Optional[Dict[str, Any]] = None):
        """Add a command to the current transaction"""
        
        if not self.active_transaction:
            raise RuntimeError("No active transaction")
        
        transaction = self.transactions[self.active_transaction]
        
        command = UECommand(
            id=f"tx_{self.active_transaction}_{len(transaction.commands)}",
            command_type=command_type,
            params=params
        )
        
        rollback_command = None
        if rollback_command_type:
            rollback_command = UECommand(
                id=f"rollback_{command.id}",
                command_type=rollback_command_type,
                params=rollback_params or {}
            )
        
        transaction.add_command(command, rollback_command)
    
    async def commit_transaction(self) -> List[CommandResult]:
        """Commit the current transaction"""
        
        if not self.active_transaction:
            raise RuntimeError("No active transaction")
        
        transaction = self.transactions[self.active_transaction]
        transaction.status = TransactionStatus.RUNNING
        
        try:
            # Execute all commands in the transaction
            results = await self._execute_commands_sequential(transaction.commands)
            transaction.results = results
            
            # Check if all commands succeeded
            if all(result.success for result in results):
                transaction.status = TransactionStatus.COMMITTED
                self.logger.info(f"Transaction {self.active_transaction} committed successfully")
            else:
                # Some commands failed, rollback
                await self._rollback_transaction(transaction)
                transaction.status = TransactionStatus.FAILED
                self.logger.error(f"Transaction {self.active_transaction} failed, rolled back")
            
            self.active_transaction = None
            return results
            
        except Exception as e:
            # Exception during execution, rollback
            await self._rollback_transaction(transaction)
            transaction.status = TransactionStatus.FAILED
            self.active_transaction = None
            self.logger.error(f"Transaction {self.active_transaction} failed with exception: {e}")
            raise
    
    async def rollback_transaction(self) -> bool:
        """Manually rollback the current transaction"""
        
        if not self.active_transaction:
            return False
        
        transaction = self.transactions[self.active_transaction]
        success = await self._rollback_transaction(transaction)
        
        transaction.status = TransactionStatus.ROLLED_BACK
        self.active_transaction = None
        
        return success

    async def _initialize_connection_pool(self):
        """Initialize the connection pool"""

        async with self.pool_lock:
            for i in range(self.max_connections):
                try:
                    reader, writer = await asyncio.wait_for(
                        asyncio.open_connection(self.host, self.port),
                        timeout=self.connection_timeout
                    )
                    self.connection_pool.append((reader, writer))
                    self.logger.debug(f"Created connection {i+1}/{self.max_connections}")
                except Exception as e:
                    self.logger.warning(f"Failed to create connection {i+1}: {e}")

    async def _get_connection(self) -> Optional[Tuple[asyncio.StreamReader, asyncio.StreamWriter]]:
        """Get a connection from the pool"""

        async with self.pool_lock:
            if self.connection_pool:
                return self.connection_pool.pop()

        # Pool is empty, create new connection
        try:
            return await asyncio.wait_for(
                asyncio.open_connection(self.host, self.port),
                timeout=self.connection_timeout
            )
        except Exception as e:
            self.logger.error(f"Failed to create new connection: {e}")
            return None

    async def _return_connection(self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
        """Return a connection to the pool"""

        async with self.pool_lock:
            if len(self.connection_pool) < self.max_connections:
                self.connection_pool.append((reader, writer))
            else:
                # Pool is full, close the connection
                writer.close()
                await writer.wait_closed()

    async def _close_all_connections(self):
        """Close all connections in the pool"""

        async with self.pool_lock:
            for reader, writer in self.connection_pool:
                writer.close()
                await writer.wait_closed()
            self.connection_pool.clear()

    async def _execute_single_command(self, command: UECommand) -> CommandResult:
        """Execute a single command with retry logic"""

        start_time = time.time()

        for attempt in range(command.max_retries + 1):
            try:
                # Get connection
                connection = await self._get_connection()
                if not connection:
                    raise Exception("Failed to get connection")

                reader, writer = connection

                # Send command
                command_json = json.dumps(command.to_dict())
                writer.write(command_json.encode('utf-8'))
                writer.write(b'\n')
                await writer.drain()

                # Read response with timeout
                response_data = await asyncio.wait_for(
                    reader.read(8192),
                    timeout=command.timeout
                )

                # Return connection to pool
                await self._return_connection(reader, writer)

                # Parse response
                response_str = response_data.decode('utf-8').strip()
                if response_str:
                    response = json.loads(response_str)

                    execution_time = time.time() - start_time
                    self._update_performance_stats(execution_time, True)

                    return CommandResult(
                        command_id=command.id,
                        success=response.get("success", False),
                        data=response if response.get("success", False) else None,
                        error=response.get("error") if not response.get("success", False) else None,
                        execution_time=execution_time,
                        retry_count=attempt
                    )
                else:
                    raise Exception("Empty response from UE")

            except Exception as e:
                self.logger.warning(f"Command {command.id} attempt {attempt + 1} failed: {e}")
                command.retry_count = attempt

                if attempt < command.max_retries:
                    await asyncio.sleep(1.0 * (attempt + 1))  # Exponential backoff
                    continue
                else:
                    # All retries exhausted
                    execution_time = time.time() - start_time
                    self._update_performance_stats(execution_time, False)

                    return CommandResult(
                        command_id=command.id,
                        success=False,
                        error=f"Command failed after {command.max_retries + 1} attempts: {str(e)}",
                        execution_time=execution_time,
                        retry_count=attempt
                    )

    async def _execute_commands_sequential(self, commands: List[UECommand]) -> List[CommandResult]:
        """Execute commands sequentially"""

        results = []
        for command in commands:
            result = await self._execute_single_command(command)
            results.append(result)

            # Stop on first failure if in transaction mode
            if not result.success and self.active_transaction:
                break

        return results

    async def _execute_commands_parallel(self, commands: List[UECommand]) -> List[CommandResult]:
        """Execute commands in parallel"""

        tasks = [self._execute_single_command(cmd) for cmd in commands]
        results = await asyncio.gather(*tasks, return_exceptions=True)

        # Convert exceptions to error results
        final_results = []
        for i, result in enumerate(results):
            if isinstance(result, Exception):
                final_results.append(CommandResult(
                    command_id=commands[i].id,
                    success=False,
                    error=str(result),
                    execution_time=0.0
                ))
            else:
                final_results.append(result)

        return final_results

    async def _rollback_transaction(self, transaction: Transaction) -> bool:
        """Execute rollback commands for a transaction"""

        if not transaction.rollback_commands:
            self.logger.info(f"No rollback commands for transaction {transaction.id}")
            return True

        self.logger.info(f"Rolling back transaction {transaction.id}")

        # Execute rollback commands in reverse order
        rollback_results = []
        for command in reversed(transaction.rollback_commands):
            result = await self._execute_single_command(command)
            rollback_results.append(result)

            if not result.success:
                self.logger.error(f"Rollback command failed: {result.error}")

        # Check if all rollback commands succeeded
        success = all(result.success for result in rollback_results)

        if success:
            self.logger.info(f"Transaction {transaction.id} rolled back successfully")
        else:
            self.logger.error(f"Transaction {transaction.id} rollback had failures")

        return success

    async def _process_command_queue(self):
        """Process commands from the queue"""

        while True:
            try:
                # Get command from queue with timeout
                command = await asyncio.wait_for(
                    self.command_queue.get(),
                    timeout=1.0
                )

                # Execute command
                result = await self._execute_single_command(command)

                # Mark task as done
                self.command_queue.task_done()

            except asyncio.TimeoutError:
                continue
            except Exception as e:
                self.logger.error(f"Error processing command queue: {e}")

    def _update_performance_stats(self, execution_time: float, success: bool):
        """Update performance statistics"""

        self.performance_stats["commands_executed"] += 1
        self.performance_stats["total_execution_time"] += execution_time

        if not success:
            self.performance_stats["failed_commands"] += 1

        # Calculate average response time
        if self.performance_stats["commands_executed"] > 0:
            self.performance_stats["average_response_time"] = (
                self.performance_stats["total_execution_time"] /
                self.performance_stats["commands_executed"]
            )

    def get_performance_stats(self) -> Dict[str, Any]:
        """Get performance statistics"""

        stats = self.performance_stats.copy()
        stats["success_rate"] = (
            (stats["commands_executed"] - stats["failed_commands"]) /
            max(stats["commands_executed"], 1)
        )
        stats["active_connections"] = len(self.connection_pool)
        stats["active_transaction"] = self.active_transaction
        stats["queued_commands"] = self.command_queue.qsize()

        return stats

    def get_transaction_status(self, transaction_id: str) -> Optional[Dict[str, Any]]:
        """Get status of a specific transaction"""

        if transaction_id not in self.transactions:
            return None

        transaction = self.transactions[transaction_id]

        return {
            "id": transaction.id,
            "status": transaction.status.value,
            "command_count": len(transaction.commands),
            "completed_commands": len(transaction.results),
            "success_rate": (
                sum(1 for r in transaction.results if r.success) /
                max(len(transaction.results), 1)
            ),
            "created_at": transaction.created_at,
            "duration": time.time() - transaction.created_at
        }

    async def queue_command(self, command_type: str, params: Dict[str, Any],
                          priority: int = 0) -> str:
        """Queue a command for asynchronous execution"""

        command = UECommand(
            id=str(uuid.uuid4()),
            command_type=command_type,
            params=params
        )

        await self.command_queue.put(command)
        return command.id

    async def wait_for_queue_empty(self, timeout: Optional[float] = None):
        """Wait for the command queue to be empty"""

        if timeout:
            await asyncio.wait_for(self.command_queue.join(), timeout=timeout)
        else:
            await self.command_queue.join()
