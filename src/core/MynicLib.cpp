#include "MynicLib.h"

double getAsDouble(Value& v) {
    auto result = std::visit([&](auto&& b) -> double {
        using B = std::decay_t<decltype(b)>;
        if constexpr ((std::is_arithmetic_v<B>)) {
            return static_cast<double>(b);
        } else {
            return -1.0;
        }
    }, v);
    return result;
}


Value MynicLib::mathMax(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter) {
    uint64_t maxNum = 0;
    double maxDouble = 0;
    for (auto& param : functionCall->parameters) {
        Value paramValue = interpreter->evaluateASTExpression(interpretedPrimitive, param);
        std::visit([&](auto&& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_integral_v<T>) {
                maxNum = std::max(maxNum, static_cast<uint64_t>(val));
            } else if constexpr (std::is_floating_point_v<T>) {
                maxDouble = std::max(maxDouble, static_cast<double>(val));
            } else {
                interpreter->throwWarning(InterpreterWarningCodes::VARIABLE_NOT_ARITHMETIC, "Parameters must be a numeric type in Math.max()");
            }
        }, paramValue);
    }
    return maxNum > maxDouble ? maxNum : maxDouble;
}

Value MynicLib::mathMin(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter) {
    uint64_t minNum = 0xFFFFFFFFFFFFFFFF;
    double minDouble = 0xFFFFFFFF;
    for (auto& param : functionCall->parameters) {
        Value paramValue = interpreter->evaluateASTExpression(interpretedPrimitive, param);
        std::visit([&](auto&& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_integral_v<T>) {
                minNum = std::min(minNum, static_cast<uint64_t>(val));
            } else if constexpr (std::is_floating_point_v<T>) {
                minDouble = std::min(minDouble, static_cast<double>(val));
            } else {
                interpreter->throwWarning(InterpreterWarningCodes::VARIABLE_NOT_ARITHMETIC, "Parameters must be a numeric type in Math.min()");
            }
        }, paramValue);
    }
    return minNum < minDouble ? minNum : minDouble;
}

Value MynicLib::mathPow(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter) {
    if (functionCall->parameters.size() != 2) {
        interpreter->throwWarning(InterpreterWarningCodes::WRONG_NUMBER_OF_PARAMETERS, "Wrong number of params in Math.pow(), expected 2 but got " + std::to_string(functionCall->parameters.size()));
        return 0;
    }
    Value base = interpreter->evaluateASTExpression(interpretedPrimitive, functionCall->parameters[0]);
    Value raised = interpreter->evaluateASTExpression(interpretedPrimitive, functionCall->parameters[1]);
    return std::pow(getAsDouble(base), getAsDouble(raised));
}

Value MynicLib::mathSqrt(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter) {
    if (functionCall->parameters.size() != 1) {
        interpreter->throwWarning(InterpreterWarningCodes::WRONG_NUMBER_OF_PARAMETERS, "Wrong number of params in Math.sqrt(), expected 1 but got " + std::to_string(functionCall->parameters.size()));
        return 0;
    }
    Value base = interpreter->evaluateASTExpression(interpretedPrimitive, functionCall->parameters[0]);
    auto result = getAsDouble(base);
    return result;
}

Value MynicLib::mathLog(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter, uint8_t logBase) {
    if (functionCall->parameters.size() != 1) {
        interpreter->throwWarning(InterpreterWarningCodes::WRONG_NUMBER_OF_PARAMETERS, "Wrong number of params in Math.log(), expected 1 but got " + std::to_string(functionCall->parameters.size()));
        return 0;
    }
    Value base = interpreter->evaluateASTExpression(interpretedPrimitive, functionCall->parameters[0]);
    auto result = getAsDouble(base);
    if (logBase == 10) return std::log10(result);
    if (logBase == 2) return std::log2(result);
    return std::log(result);
}

Value MynicLib::mathRound(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter) {
    if (functionCall->parameters.size() != 1) {
        interpreter->throwWarning(InterpreterWarningCodes::WRONG_NUMBER_OF_PARAMETERS, "Wrong number of params in Math.round(), expected 1 but got " + std::to_string(functionCall->parameters.size()));
        return 0;
    }
    Value base = interpreter->evaluateASTExpression(interpretedPrimitive, functionCall->parameters[0]);
    auto result = getAsDouble(base);
    return std::round(result);
}

Value MynicLib::mathSign(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter) {
    if (functionCall->parameters.size() != 1) {
        interpreter->throwWarning(InterpreterWarningCodes::WRONG_NUMBER_OF_PARAMETERS, "Wrong number of params in Math.sign(), expected 1 but got " + std::to_string(functionCall->parameters.size()));
        return 0;
    }
    Value base = interpreter->evaluateASTExpression(interpretedPrimitive, functionCall->parameters[0]);
    auto result = getAsDouble(base);
    if (result < 0) return -1;
    if (result > 0) return 1;
    return 0;
}

Value MynicLib::mathAbs(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter) {
    if (functionCall->parameters.size() != 1) {
        interpreter->throwWarning(InterpreterWarningCodes::WRONG_NUMBER_OF_PARAMETERS, "Wrong number of params in Math.abs(), expected 1 but got " + std::to_string(functionCall->parameters.size()));
        return 0;
    }
    Value base = interpreter->evaluateASTExpression(interpretedPrimitive, functionCall->parameters[0]);
    auto result = getAsDouble(base);
    return std::abs(result);
}

Value MynicLib::mathPi() {return M_PI;}
Value MynicLib::mathE() {return M_E;}

void MynicLib::rewind(std::shared_ptr<ASTFunctionCall> functionCall, Interpreter* interpreter) {
    if (functionCall->parameters.size() != 1) {
        interpreter->throwWarning(InterpreterWarningCodes::WRONG_NUMBER_OF_PARAMETERS, "Wrong number of params in REWIND(), expected 1 but got " + std::to_string(functionCall->parameters.size()));
        return;
    }
    Value bitVal = interpreter->evaluateASTExpression(nullptr, functionCall->parameters[0]);
    size_t bitNum = static_cast<size_t>(getAsDouble(bitVal));
    if (interpreter->bitQueue.bitPos() < bitNum) {
        interpreter->throwWarning(InterpreterWarningCodes::BIT_QUEUE_INDEX_OUT_OF_BOUNDS, "Bit index out of bounds in REWIND()");
        return;
    }
    interpreter->bitQueue.rewind(bitNum);
}

void MynicLib::skip(std::shared_ptr<ASTFunctionCall> functionCall, Interpreter* interpreter) {
    if (functionCall->parameters.size() != 1) {
        interpreter->throwWarning(InterpreterWarningCodes::WRONG_NUMBER_OF_PARAMETERS, "Wrong number of params in SKIP(), expected 1 but got " + std::to_string(functionCall->parameters.size()));
        return;
    }
    Value bitVal = interpreter->evaluateASTExpression(nullptr, functionCall->parameters[0]);
    size_t bitNum = static_cast<size_t>(getAsDouble(bitVal));
    if (interpreter->bitQueue.size() < bitNum) {
        interpreter->throwWarning(InterpreterWarningCodes::BIT_QUEUE_INDEX_OUT_OF_BOUNDS, "Bit index out of bounds in SKIP()");
        return;
    }
    interpreter->bitQueue.pop(bitNum);
}

void MynicLib::seek(std::shared_ptr<ASTFunctionCall> functionCall, Interpreter* interpreter) {
    if (functionCall->parameters.size() != 1) {
        interpreter->throwWarning(InterpreterWarningCodes::WRONG_NUMBER_OF_PARAMETERS, "Wrong number of params in SEEK(), expected 1 but got " + std::to_string(functionCall->parameters.size()));
        return;
    }
    Value bitVal = interpreter->evaluateASTExpression(nullptr, functionCall->parameters[0]);
    size_t bitPos = static_cast<size_t>(getAsDouble(bitVal));
    if (interpreter->bitQueue.size() >= bitPos) {
        interpreter->throwWarning(InterpreterWarningCodes::BIT_QUEUE_INDEX_OUT_OF_BOUNDS, "Bit index out of bounds in SEEK()");
        return;
    }
    interpreter->bitQueue.setBitPos(bitPos);
}

void MynicLib::terminateIf(std::shared_ptr<ASTFunctionCall> functionCall, Interpreter* interpreter) {
    if (functionCall->parameters.size() != 1) {
        interpreter->throwWarning(InterpreterWarningCodes::WRONG_NUMBER_OF_PARAMETERS, "Wrong number of params in TERMINATE_IF(), expected 1 but got " + std::to_string(functionCall->parameters.size()));
        return;
    }
    bool evaluatedCondition = interpreter->evaluateASTCondition(nullptr, functionCall->parameters[0]);
    interpreter->terminateSignal = evaluatedCondition;
}