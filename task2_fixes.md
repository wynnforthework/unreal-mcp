# Task 2 Fixes - Проблеми з MCP плагіном

## Основні проблеми виявлені:

### 1. Timeout проблеми
- TCP з'єднання працює, але є timeout'и при створенні нодів
- Це може бути через складність операцій або проблеми з C++ кодом

### 2. Відсутність функціональності для створення констант
- Немає способу створити float константу (0.0) для InRange_FloatFloat.Min
- Потрібно додати підтримку literal значень

### 3. Неповна логіка CanInteract
- Не вистачає параметра OtherActor для GetDistanceTo
- CustomEvent замість Function
- Немає return value

## Рекомендовані фікси:

### 1. Додати підтримку literal значень
```cpp
// В BlueprintNodeCreationService.cpp додати:
else if (EffectiveFunctionName.StartsWith(TEXT("Literal")) || 
         EffectiveFunctionName.Equals(TEXT("Float"), ESearchCase::IgnoreCase))
{
    // Створити K2Node_Literal для float/int/bool значень
    UK2Node_Literal* LiteralNode = NewObject<UK2Node_Literal>(EventGraph);
    // Встановити значення з параметрів
    // ...
}
```

### 2. Покращити timeout handling
```cpp
// В UnrealConnection збільшити timeout:
self.socket.settimeout(30)  // Збільшити до 30 секунд
```

### 3. Додати підтримку Function створення
```cpp
// Додати можливість створювати Blueprint Functions замість тільки CustomEvents
else if (EffectiveFunctionName.Equals(TEXT("Function"), ESearchCase::IgnoreCase))
{
    // Створити нову функцію в Blueprint
    UEdGraph* FunctionGraph = FBlueprintEditorUtils::CreateNewGraph(...);
    // ...
}
```

### 4. Покращити error handling
- Додати більше діагностичної інформації
- Покращити логування для debug'у
- Додати fallback механізми

## Висновок:
Основна проблема не в логіці task 2, а в тому що MCP плагін не має достатньої функціональності для створення простих констант та має проблеми з timeout'ами. Потрібно покращити сам плагін.