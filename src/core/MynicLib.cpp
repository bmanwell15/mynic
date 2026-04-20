#include "MynicLib.h"

double getAsDouble(Value& v) {
    auto result = std::visit([&](auto&& b) -> double {
        using B = std::decay_t<decltype(b)>;
        if constexpr ((std::is_arithmetic_v<B>)) {
            return static_cast<double>(b);
        } else {
            throw std::runtime_error("Unsupported type: both arguments must be numeric");
        }
    }, v);
    return result;
}


Value MynicLib::mathMax(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTFunctionCall> functionCall, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode, Interpreter* interpreter) {
    uint64_t maxNum = 0;
    double maxDouble = 0;
    for (auto& param : functionCall->parameters) {
        Value paramValue = interpreter->evaluateASTExpression(interpretedPrimitive, param, bitQueue, rootNode);
        std::visit([&](auto&& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_integral_v<T>) {
                maxNum = std::max(maxNum, static_cast<uint64_t>(val));
            } else if constexpr (std::is_floating_point_v<T>) {
                maxDouble = std::max(maxDouble, static_cast<double>(val));
            } else {
                throw std::runtime_error("Unsupported type");
            }
        }, paramValue);
    }
    return maxNum > maxDouble ? maxNum : maxDouble;
}

Value MynicLib::mathMin(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTFunctionCall> functionCall, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode, Interpreter* interpreter) {
    uint64_t minNum = 0xFFFFFFFFFFFFFFFF;
    double minDouble = 0xFFFFFFFF;
    for (auto& param : functionCall->parameters) {
        Value paramValue = interpreter->evaluateASTExpression(interpretedPrimitive, param, bitQueue, rootNode);
        std::visit([&](auto&& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_integral_v<T>) {
                minNum = std::min(minNum, static_cast<uint64_t>(val));
            } else if constexpr (std::is_floating_point_v<T>) {
                minDouble = std::min(minDouble, static_cast<double>(val));
            } else {
                throw std::runtime_error("Unsupported type");
            }
        }, paramValue);
    }
    return minNum < minDouble ? minNum : minDouble;
}

Value MynicLib::mathPow(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTFunctionCall> functionCall, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode, Interpreter* interpreter) {
    if (functionCall->parameters.size() != 2) {throw::std::runtime_error("Wrong number of params in Math.pow()");}
    Value base = interpreter->evaluateASTExpression(interpretedPrimitive, functionCall->parameters[0], bitQueue, rootNode);
    Value raised = interpreter->evaluateASTExpression(interpretedPrimitive, functionCall->parameters[1], bitQueue, rootNode);
    return std::pow(getAsDouble(base), getAsDouble(raised));
}

Value MynicLib::mathSqrt(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTFunctionCall> functionCall, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode, Interpreter* interpreter) {
    if (functionCall->parameters.size() != 1) {throw::std::runtime_error("Wrong number of params in Math.sqrt()");}
    Value base = interpreter->evaluateASTExpression(interpretedPrimitive, functionCall->parameters[0], bitQueue, rootNode);
    auto result = getAsDouble(base);
    return result;
}

Value MynicLib::mathLog(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTFunctionCall> functionCall, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode, Interpreter* interpreter, uint8_t logBase) {
    if (functionCall->parameters.size() != 1) {throw::std::runtime_error("Wrong number of params in Math.log()");}
    Value base = interpreter->evaluateASTExpression(interpretedPrimitive, functionCall->parameters[0], bitQueue, rootNode);
    auto result = getAsDouble(base);
    if (logBase == 10) return std::log10(result);
    if (logBase == 2) return std::log2(result);
    return std::log(result);
}

Value MynicLib::mathRound(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTFunctionCall> functionCall, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode, Interpreter* interpreter) {
    if (functionCall->parameters.size() != 1) {throw::std::runtime_error("Wrong number of params in Math.round()");}
    Value base = interpreter->evaluateASTExpression(interpretedPrimitive, functionCall->parameters[0], bitQueue, rootNode);
    auto result = getAsDouble(base);
    return std::round(result);
}

Value MynicLib::mathSign(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTFunctionCall> functionCall, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode, Interpreter* interpreter) {
    if (functionCall->parameters.size() != 1) {throw::std::runtime_error("Wrong number of params in Math.sign()");}
    Value base = interpreter->evaluateASTExpression(interpretedPrimitive, functionCall->parameters[0], bitQueue, rootNode);
    auto result = getAsDouble(base);
    if (result < 0) return -1;
    if (result > 0) return 1;
    return 0;
}

Value MynicLib::mathAbs(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTFunctionCall> functionCall, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode, Interpreter* interpreter) {
    if (functionCall->parameters.size() != 1) {throw::std::runtime_error("Wrong number of params in Math.sign()");}
    Value base = interpreter->evaluateASTExpression(interpretedPrimitive, functionCall->parameters[0], bitQueue, rootNode);
    auto result = getAsDouble(base);
    return std::abs(result);
}

Value MynicLib::mathPi() {return M_PI;}
Value MynicLib::mathE() {return M_E;}