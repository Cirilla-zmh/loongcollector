/*
 * Copyright 2024 iLogtail Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugin/processor/inner/ProcessorParseSpanNative.h"
#include "models/PipelineEventGroup.h"
#include "protobuf/models/span_event.pb.h"
#include "unittest/Unittest.h"

using namespace std;

namespace logtail {

class ProcessorParseSpanNativeUnittest : public testing::Test {
public:
    void SetUp() override {}

    void TestInit();
    void TestProcessValidSpanData();
    void TestProcessInvalidSpanData();

private:
    void generateHttpServerValidSpanData(PipelineEventGroup&);
    void generateNoSQLValidSpanData(PipelineEventGroup&);

    void assertHttpServerValidSpanData(const PipelineEventPtr&);
    void assertNoSQLValidSpanData(const PipelineEventPtr&);
};

void ProcessorParseSpanNativeUnittest::TestInit() {
    Json::Value config;
    ProcessorParseSpanNative processor;

    // Init always returns true in current implementation
    APSARA_TEST_TRUE(processor.Init(config));
}

void ProcessorParseSpanNativeUnittest::TestProcessValidSpanData() {
    ProcessorParseSpanNative processor;

    // Prepare event group with raw span data
    PipelineEventGroup eventGroup(std::make_shared<SourceBuffer>());
    this->generateHttpServerValidSpanData(eventGroup);
    this->generateNoSQLValidSpanData(eventGroup);

    // Process the event
    APSARA_TEST_EQUAL((size_t)2, eventGroup.GetEvents().size());
    processor.Process(eventGroup);

    // Validate output
    APSARA_TEST_EQUAL((size_t)2, eventGroup.GetEvents().size());
    this->assertHttpServerValidSpanData(eventGroup.GetEvents()[0]);
    this->assertNoSQLValidSpanData(eventGroup.GetEvents()[1]);
}

void ProcessorParseSpanNativeUnittest::TestProcessInvalidSpanData() {
    // TODO
}

void ProcessorParseSpanNativeUnittest::generateHttpServerValidSpanData(PipelineEventGroup& eventGroup) {
    models::SpanEvent pbSpan;

    pbSpan.set_traceid("cba78930fe0c2626bc60696a3453cc40");
    pbSpan.set_spanid("4083239a6a2e704e");
    pbSpan.set_parentspanid("d42788c106b9c48e");
    pbSpan.set_name("/components/api/v1/http/success");
    pbSpan.set_kind(models::SpanEvent::SERVER); // kind=2 is SERVER
    pbSpan.set_starttime(1748313835253000000ULL);
    pbSpan.set_endtime(1748313840262969241ULL);
    pbSpan.set_status(models::SpanEvent::Unset); // statusCode=0

    std::map<std::string, std::string> mergedTags1 = {
        {"http.path", "/components/api/v1/http/success"},
        {"endpoint", "mall-user-service:9190"},
        {"http.method", "POST"},
        {"component.name", "http"},
        {"http.status_code", "200"},
        {"http.route", "/components/api/v1/http/success"}
    };

    for (const auto& tag : mergedTags1) {
        (*pbSpan.mutable_tags())[tag.first] = tag.second;
    }

    std::map<std::string, std::string> mergedScopeTags1 = {
        {"otel.scope.version", "1.28.0-alpha"},
        {"otel.scope.name", "io.opentelemetry.tomcat-8.0.15"}
    };

    for (const auto& tag : mergedScopeTags1) {
        (*pbSpan.mutable_scopetags())[tag.first] = tag.second;
    }

    eventGroup.AddRawEvent()->SetContent(pbSpan.SerializeAsString());
}

void ProcessorParseSpanNativeUnittest::generateNoSQLValidSpanData(PipelineEventGroup& eventGroup) {
    models::SpanEvent pbSpan;

    pbSpan.set_traceid("cba78930fe0c2626bc60696a3453cc40");
    pbSpan.set_spanid("9a2c1a8a371d6798");
    pbSpan.set_parentspanid("4083239a6a2e704e");
    pbSpan.set_name("LLEN");
    pbSpan.set_kind(models::SpanEvent::CLIENT); // kind=2 is SERVER
    pbSpan.set_starttime(1748313840259486017ULL);
    pbSpan.set_endtime(1748313840259765375ULL);
    pbSpan.set_status(models::SpanEvent::Unset); // statusCode=0

    std::map<std::string, std::string> mergedTags2 = {
        {"db.system", "redis"},
        {"endpoint", "redis:6379"},
        {"component.name", "redis"},
        {"db.name", "redis:6379"},
        {"net.peer.name", "redis:6379"},
        {"redis.args", "key<big_key>"},
        {"db.statement.id", "2191aada7df3c872"}
    };

    for (const auto& tag : mergedTags2) {
        (*pbSpan.mutable_tags())[tag.first] = tag.second;
    }

    std::map<std::string, std::string> mergedScopeTags2 = {
        {"otel.scope.version", "1.28.0-alpha"},
        {"otel.scope.name", "io.opentelemetry.lettuce-5.1"}
    };

    for (const auto& tag : mergedScopeTags2) {
        (*pbSpan.mutable_scopetags())[tag.first] = tag.second;
    }
    
    eventGroup.AddRawEvent()->SetContent(pbSpan.SerializeAsString());
}

void ProcessorParseSpanNativeUnittest::assertHttpServerValidSpanData(const PipelineEventPtr& event) {
    APSARA_TEST_TRUE(event.Is<SpanEvent>());
    const auto& spanEvent = event.Cast<SpanEvent>();

    APSARA_TEST_EQUAL("cba78930fe0c2626bc60696a3453cc40", spanEvent.GetTraceId());
    APSARA_TEST_EQUAL("4083239a6a2e704e", spanEvent.GetSpanId());
    APSARA_TEST_EQUAL("d42788c106b9c48e", spanEvent.GetParentSpanId());
    APSARA_TEST_EQUAL("/components/api/v1/http/success", spanEvent.GetName());
    APSARA_TEST_EQUAL(SpanEvent::Kind::Client, spanEvent.GetKind());
    APSARA_TEST_EQUAL(1748313835253000000ULL, spanEvent.GetStartTimeNs());
    APSARA_TEST_EQUAL(1748313840262969241ULL, spanEvent.GetEndTimeNs());
    APSARA_TEST_EQUAL(SpanEvent::StatusCode::Unset, spanEvent.GetStatus());

    // Assert tags
    APSARA_TEST_EQUAL(6, spanEvent.TagsSize());
    APSARA_TEST_EQUAL("/components/api/v1/http/success", spanEvent.GetTag("http.path"));
    APSARA_TEST_EQUAL("mall-user-service:9190", spanEvent.GetTag("endpoint"));
    APSARA_TEST_EQUAL("POST", spanEvent.GetTag("http.method"));
    APSARA_TEST_EQUAL("http", spanEvent.GetTag("component.name"));
    APSARA_TEST_EQUAL("200", spanEvent.GetTag("http.status_code"));
    APSARA_TEST_EQUAL("/components/api/v1/http/success", spanEvent.GetTag("http.route"));

    // Assert scope tags
    APSARA_TEST_EQUAL(2, spanEvent.ScopeTagsSize());
    APSARA_TEST_EQUAL("1.28.0-alpha", spanEvent.GetTag("otel.scope.version"));
    APSARA_TEST_EQUAL("io.opentelemetry.tomcat-8.0.15", spanEvent.GetTag("otel.scope.name"));
}

void ProcessorParseSpanNativeUnittest::assertNoSQLValidSpanData(const PipelineEventPtr& event) {
    APSARA_TEST_TRUE(event.Is<SpanEvent>());
    const auto& spanEvent = event.Cast<SpanEvent>();

    APSARA_TEST_EQUAL("cba78930fe0c2626bc60696a3453cc40", spanEvent.GetTraceId());
    APSARA_TEST_EQUAL("9a2c1a8a371d6798", spanEvent.GetSpanId());
    APSARA_TEST_EQUAL("4083239a6a2e704e", spanEvent.GetParentSpanId());
    APSARA_TEST_EQUAL("LLEN", spanEvent.GetName());
    APSARA_TEST_EQUAL(SpanEvent::Kind::Client, spanEvent.GetKind());
    APSARA_TEST_EQUAL(1748313840259486017ULL, spanEvent.GetStartTimeNs());
    APSARA_TEST_EQUAL(1748313840259765375ULL, spanEvent.GetEndTimeNs());
    APSARA_TEST_EQUAL(SpanEvent::StatusCode::Unset, spanEvent.GetStatus());

    // Assert tags
    APSARA_TEST_EQUAL(7, spanEvent.TagsSize());
    APSARA_TEST_EQUAL("redis", spanEvent.GetTag("db.system"));
    APSARA_TEST_EQUAL("redis:6379", spanEvent.GetTag("endpoint"));
    APSARA_TEST_EQUAL("redis", spanEvent.GetTag("component.name"));
    APSARA_TEST_EQUAL("redis:6379", spanEvent.GetTag("db.name"));
    APSARA_TEST_EQUAL("redis:6379", spanEvent.GetTag("net.peer.name"));
    APSARA_TEST_EQUAL("key<big_key>", spanEvent.GetTag("redis.args"));
    APSARA_TEST_EQUAL("2191aada7df3c872", spanEvent.GetTag("db.statement.id"));

    // Assert scope tags
    APSARA_TEST_EQUAL(2, spanEvent.ScopeTagsSize());
    APSARA_TEST_EQUAL("1.28.0-alpha", spanEvent.GetTag("otel.scope.version"));
    APSARA_TEST_EQUAL("io.opentelemetry.lettuce-5.1", spanEvent.GetTag("otel.scope.name"));
}

UNIT_TEST_CASE(ProcessorParseSpanNativeUnittest, TestInit)
UNIT_TEST_CASE(ProcessorParseSpanNativeUnittest, TestProcessValidSpanData)
UNIT_TEST_CASE(ProcessorParseSpanNativeUnittest, TestProcessInvalidSpanData)

} // namespace logtail

UNIT_TEST_MAIN
