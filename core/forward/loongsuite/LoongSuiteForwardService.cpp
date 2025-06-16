/*
 * Copyright 2025 iLogtail Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "forward/loongsuite/LoongSuiteForwardService.h"

#include "grpcpp/support/status.h"
#include "logger/Logger.h"

namespace logtail {

const std::string BaseService::sName = "LoongSuiteForwardService";

bool LoongSuiteForwardServiceImpl::Update(std::string configName, const Json::Value& config) {
    // Initialize the service with the provided configuration
    // For now, we just log the initialization
    LOG_INFO(sLogger, ("LoongSuiteForwardServiceImpl updated with config", config.toStyledString()));
    return true; // Return true to indicate successful initialization
}

bool LoongSuiteForwardServiceImpl::Remove(std::string configName) {
    // Handle the removal of the service configuration
    // For now, we just log the removal
    LOG_INFO(sLogger, ("LoongSuiteForwardServiceImpl removed for config", configName));
    return true; // Return true to indicate successful removal
}

grpc::ServerUnaryReactor* LoongSuiteForwardServiceImpl::Forward(grpc::CallbackServerContext* context,
                                                                const LoongSuiteForwardRequest* request,
                                                                LoongSuiteForwardResponse* response) {
    if (context == nullptr || request == nullptr || response == nullptr) {
        LOG_ERROR(sLogger, ("Invalid parameters in Forward method", "null pointer detected"));
        auto* reactor = context ? context->DefaultReactor() : nullptr;
        if (reactor) {
            reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Invalid parameters"));
        }
        return reactor;
    }

    // For now, we just read and print data in request
    const std::string& data = request->data();
    printData(data);

    auto* reactor = context->DefaultReactor();
    reactor->Finish(grpc::Status::OK);
    return reactor;
}

void LoongSuiteForwardServiceImpl::printData(const std::string& data) {
    LOG_INFO(sLogger, ("Received data size", data.size()));
    std::string byteValues;
    for (const auto& byte : data) {
        byteValues += std::to_string(static_cast<int>(static_cast<unsigned char>(byte))) + " ";
    }
    LOG_INFO(sLogger, ("Data bytes", byteValues));
}

} // namespace logtail
