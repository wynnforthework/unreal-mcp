"""
Agent Communication Protocols

Defines the communication layer between agents, including message passing,
state management, and data validation for the UMG generation workflow.
"""

import asyncio
import json
import uuid
from dataclasses import dataclass, field
from datetime import datetime
from typing import Any, Dict, List, Optional, Callable, Awaitable
from enum import Enum
import logging


class MessageType(Enum):
    """Types of messages exchanged between agents"""
    REQUEST = "request"
    RESPONSE = "response"
    ERROR = "error"
    STATUS_UPDATE = "status_update"
    HEARTBEAT = "heartbeat"


class MessagePriority(Enum):
    """Message priority levels"""
    LOW = 1
    NORMAL = 2
    HIGH = 3
    CRITICAL = 4


@dataclass
class AgentMessage:
    """Message structure for agent communication"""
    id: str = field(default_factory=lambda: str(uuid.uuid4()))
    type: MessageType = MessageType.REQUEST
    sender: str = ""
    recipient: str = ""
    payload: Dict[str, Any] = field(default_factory=dict)
    priority: MessagePriority = MessagePriority.NORMAL
    timestamp: datetime = field(default_factory=datetime.now)
    correlation_id: Optional[str] = None
    reply_to: Optional[str] = None
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "id": self.id,
            "type": self.type.value,
            "sender": self.sender,
            "recipient": self.recipient,
            "payload": self.payload,
            "priority": self.priority.value,
            "timestamp": self.timestamp.isoformat(),
            "correlation_id": self.correlation_id,
            "reply_to": self.reply_to
        }
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'AgentMessage':
        return cls(
            id=data["id"],
            type=MessageType(data["type"]),
            sender=data["sender"],
            recipient=data["recipient"],
            payload=data["payload"],
            priority=MessagePriority(data["priority"]),
            timestamp=datetime.fromisoformat(data["timestamp"]),
            correlation_id=data.get("correlation_id"),
            reply_to=data.get("reply_to")
        )


class MessageBus:
    """
    Message bus for agent communication.
    
    Handles message routing, queuing, and delivery between agents
    in the UMG generation workflow.
    """
    
    def __init__(self):
        self.agents: Dict[str, 'AgentEndpoint'] = {}
        self.message_queue: asyncio.Queue = asyncio.Queue()
        self.message_handlers: Dict[str, List[Callable]] = {}
        self.logger = logging.getLogger("message_bus")
        self._running = False
        self._processor_task: Optional[asyncio.Task] = None
    
    async def start(self):
        """Start the message bus processor"""
        if self._running:
            return
        
        self._running = True
        self._processor_task = asyncio.create_task(self._process_messages())
        self.logger.info("Message bus started")
    
    async def stop(self):
        """Stop the message bus processor"""
        if not self._running:
            return
        
        self._running = False
        if self._processor_task:
            self._processor_task.cancel()
            try:
                await self._processor_task
            except asyncio.CancelledError:
                pass
        
        self.logger.info("Message bus stopped")
    
    def register_agent(self, agent_id: str, endpoint: 'AgentEndpoint'):
        """Register an agent with the message bus"""
        self.agents[agent_id] = endpoint
        self.logger.info(f"Agent {agent_id} registered")
    
    def unregister_agent(self, agent_id: str):
        """Unregister an agent from the message bus"""
        if agent_id in self.agents:
            del self.agents[agent_id]
            self.logger.info(f"Agent {agent_id} unregistered")
    
    async def send_message(self, message: AgentMessage):
        """Send a message through the bus"""
        await self.message_queue.put(message)
    
    async def _process_messages(self):
        """Process messages from the queue"""
        while self._running:
            try:
                # Wait for message with timeout to allow checking _running flag
                message = await asyncio.wait_for(
                    self.message_queue.get(), 
                    timeout=1.0
                )
                await self._deliver_message(message)
            except asyncio.TimeoutError:
                continue
            except Exception as e:
                self.logger.error(f"Error processing message: {e}")
    
    async def _deliver_message(self, message: AgentMessage):
        """Deliver a message to the target agent"""
        recipient = message.recipient
        
        if recipient not in self.agents:
            self.logger.error(f"Agent {recipient} not found for message {message.id}")
            return
        
        try:
            await self.agents[recipient].receive_message(message)
            self.logger.debug(f"Message {message.id} delivered to {recipient}")
        except Exception as e:
            self.logger.error(f"Failed to deliver message {message.id} to {recipient}: {e}")


class AgentEndpoint:
    """
    Communication endpoint for an agent.
    
    Handles sending and receiving messages for a specific agent.
    """
    
    def __init__(self, agent_id: str, message_bus: MessageBus):
        self.agent_id = agent_id
        self.message_bus = message_bus
        self.message_handlers: Dict[MessageType, List[Callable]] = {}
        self.logger = logging.getLogger(f"endpoint.{agent_id}")
        
        # Register with message bus
        message_bus.register_agent(agent_id, self)
    
    def add_message_handler(self, message_type: MessageType, 
                          handler: Callable[[AgentMessage], Awaitable[None]]):
        """Add a message handler for a specific message type"""
        if message_type not in self.message_handlers:
            self.message_handlers[message_type] = []
        self.message_handlers[message_type].append(handler)
    
    async def send_message(self, recipient: str, payload: Dict[str, Any],
                          message_type: MessageType = MessageType.REQUEST,
                          priority: MessagePriority = MessagePriority.NORMAL,
                          correlation_id: Optional[str] = None) -> str:
        """Send a message to another agent"""
        message = AgentMessage(
            type=message_type,
            sender=self.agent_id,
            recipient=recipient,
            payload=payload,
            priority=priority,
            correlation_id=correlation_id
        )
        
        await self.message_bus.send_message(message)
        return message.id
    
    async def send_response(self, original_message: AgentMessage, 
                          payload: Dict[str, Any],
                          message_type: MessageType = MessageType.RESPONSE):
        """Send a response to a received message"""
        response = AgentMessage(
            type=message_type,
            sender=self.agent_id,
            recipient=original_message.sender,
            payload=payload,
            correlation_id=original_message.id,
            reply_to=original_message.id
        )
        
        await self.message_bus.send_message(response)
    
    async def receive_message(self, message: AgentMessage):
        """Receive and handle a message"""
        self.logger.debug(f"Received message {message.id} from {message.sender}")
        
        # Call registered handlers for this message type
        handlers = self.message_handlers.get(message.type, [])
        for handler in handlers:
            try:
                await handler(message)
            except Exception as e:
                self.logger.error(f"Error in message handler: {e}")
                
                # Send error response if this was a request
                if message.type == MessageType.REQUEST:
                    await self.send_response(
                        message,
                        {"error": str(e)},
                        MessageType.ERROR
                    )


class WorkflowState:
    """
    Manages the state of the agent workflow.
    
    Tracks progress, handles state transitions, and provides
    rollback capabilities for the UMG generation process.
    """
    
    def __init__(self, workflow_id: str):
        self.workflow_id = workflow_id
        self.current_step = 0
        self.steps: List[Dict[str, Any]] = []
        self.state_data: Dict[str, Any] = {}
        self.checkpoints: List[Dict[str, Any]] = []
        self.logger = logging.getLogger(f"workflow.{workflow_id}")
    
    def add_step(self, step_name: str, agent_id: str, input_data: Dict[str, Any]):
        """Add a step to the workflow"""
        step = {
            "name": step_name,
            "agent_id": agent_id,
            "input_data": input_data,
            "output_data": None,
            "status": "pending",
            "start_time": None,
            "end_time": None,
            "error": None
        }
        self.steps.append(step)
    
    def start_step(self, step_index: int):
        """Mark a step as started"""
        if 0 <= step_index < len(self.steps):
            self.steps[step_index]["status"] = "running"
            self.steps[step_index]["start_time"] = datetime.now()
            self.current_step = step_index
    
    def complete_step(self, step_index: int, output_data: Dict[str, Any]):
        """Mark a step as completed"""
        if 0 <= step_index < len(self.steps):
            self.steps[step_index]["status"] = "completed"
            self.steps[step_index]["output_data"] = output_data
            self.steps[step_index]["end_time"] = datetime.now()
            
            # Create checkpoint
            self.create_checkpoint()
    
    def fail_step(self, step_index: int, error: str):
        """Mark a step as failed"""
        if 0 <= step_index < len(self.steps):
            self.steps[step_index]["status"] = "failed"
            self.steps[step_index]["error"] = error
            self.steps[step_index]["end_time"] = datetime.now()
    
    def create_checkpoint(self):
        """Create a checkpoint of the current state"""
        checkpoint = {
            "timestamp": datetime.now(),
            "current_step": self.current_step,
            "state_data": self.state_data.copy(),
            "completed_steps": [i for i, step in enumerate(self.steps) 
                              if step["status"] == "completed"]
        }
        self.checkpoints.append(checkpoint)
    
    def rollback_to_checkpoint(self, checkpoint_index: int = -1):
        """Rollback to a previous checkpoint"""
        if not self.checkpoints:
            return False
        
        checkpoint = self.checkpoints[checkpoint_index]
        self.current_step = checkpoint["current_step"]
        self.state_data = checkpoint["state_data"].copy()
        
        # Reset steps after the checkpoint
        for i in range(len(self.steps)):
            if i not in checkpoint["completed_steps"]:
                self.steps[i]["status"] = "pending"
                self.steps[i]["output_data"] = None
                self.steps[i]["start_time"] = None
                self.steps[i]["end_time"] = None
                self.steps[i]["error"] = None
        
        return True
    
    def get_progress(self) -> Dict[str, Any]:
        """Get workflow progress information"""
        completed = sum(1 for step in self.steps if step["status"] == "completed")
        failed = sum(1 for step in self.steps if step["status"] == "failed")
        
        return {
            "workflow_id": self.workflow_id,
            "total_steps": len(self.steps),
            "completed_steps": completed,
            "failed_steps": failed,
            "current_step": self.current_step,
            "progress_percentage": (completed / len(self.steps)) * 100 if self.steps else 0,
            "status": self._get_overall_status()
        }
    
    def _get_overall_status(self) -> str:
        """Get the overall workflow status"""
        if any(step["status"] == "failed" for step in self.steps):
            return "failed"
        elif all(step["status"] == "completed" for step in self.steps):
            return "completed"
        elif any(step["status"] == "running" for step in self.steps):
            return "running"
        else:
            return "pending"
