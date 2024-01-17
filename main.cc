#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/utils/logging/DefaultLogSystem.h>
#include <aws/core/utils/logging/AWSLogging.h>
#include <aws/lexv2-runtime/LexRuntimeV2Client.h>
#include <aws/lexv2-runtime/model/StartConversationRequest.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "aws/core/external/cjson/cJSON.h"
#include <ctime>

using namespace Aws;
using namespace Aws::Utils;
using namespace Aws::Auth;
using namespace Aws::LexRuntimeV2;
using namespace Aws::LexRuntimeV2::Model;

char* logCurrentTime() {
    std::time_t now = std::time(0);
    char* dt = std::ctime(&now);

    return dt;
}

cJSON* lex2Json(const Message& msg) {
	cJSON * json = cJSON_AS4CPP_CreateObject();

  cJSON_AS4CPP_AddItemToObject(json, "msg", cJSON_AS4CPP_CreateString(msg.GetContent().c_str()));
  cJSON_AS4CPP_AddItemToObject(json, "type", 
    cJSON_AS4CPP_CreateString(MessageContentTypeMapper::GetNameForMessageContentType(msg.GetContentType()).c_str()));

  return json;
}

cJSON* lex2Json(const PlaybackInterruptionEvent& ev) {
  cJSON * json = cJSON_AS4CPP_CreateObject();

  cJSON_AS4CPP_AddItemToObject(json, "reason", 
    cJSON_AS4CPP_CreateString(PlaybackInterruptionReasonMapper::GetNameForPlaybackInterruptionReason(ev.GetEventReason()).c_str()));
  cJSON_AS4CPP_AddItemToObject(json, "causedBy", cJSON_AS4CPP_CreateString(ev.GetCausedByEventId().c_str()));
  cJSON_AS4CPP_AddItemToObject(json, "eventId", cJSON_AS4CPP_CreateString(ev.GetEventId().c_str()));

  return json;
}
cJSON* lex2Json(const TranscriptEvent& ev) {
	cJSON * json = cJSON_AS4CPP_CreateObject();

  cJSON_AS4CPP_AddItemToObject(json, "transcript", cJSON_AS4CPP_CreateString(ev.GetTranscript().c_str()));
  cJSON_AS4CPP_AddItemToObject(json, "eventId", cJSON_AS4CPP_CreateString(ev.GetEventId().c_str()));

  return json;
}

cJSON* lex2Json(const TextResponseEvent& ev) {
	cJSON * json = cJSON_AS4CPP_CreateObject();

  cJSON_AS4CPP_AddItemToObject(json, "eventId", cJSON_AS4CPP_CreateString(ev.GetEventId().c_str()));

	cJSON* jMessages = cJSON_AS4CPP_CreateArray();
  cJSON_AS4CPP_AddItemToObject(json, "messages", jMessages);
	for (auto msg : ev.GetMessages()) {
    cJSON_AS4CPP_AddItemToArray(jMessages, lex2Json(msg));
  }
  return json;
}
int main(int argc, char *argv[]) {
  // check env variables
  const char* botID = std::getenv("BOT_ID");
  const char* botAliasID = std::getenv("BOT_ALIAS_ID");
  const char* awsAccessKey = std::getenv("AWS_ACCESS_KEY");
  const char* awsSecretAccessKey = std::getenv("AWS_SECRET_ACCESS_KEY");
  if (botID == NULL || botAliasID == NULL || awsAccessKey == NULL || awsSecretAccessKey == NULL) {
    std::cout << "BOT_ID or BOT_ALIAS_ID or AWS_ACCESS_KEY or AWS_SECRET_ACCESS_KEY is missing, exit" << std::endl;
    return 1;
  }

  if (argc != 2) {
      std::cerr << "Usage: " << argv[0] << " <audio_file_path>" << std::endl;
      return 1;
  }

  std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
  StartConversationRequestEventStream* m_pStream;
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  std::string uuid_str = boost::uuids::to_string(uuid);
  const std::string audioFilePath = argv[1];

  std::ifstream file(audioFilePath, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
      std::cerr << "Failed to open " << audioFilePath << std::endl;
      return 1;
  }

  if (!file.is_open()) {
      std::cerr << "Failed to open I-would-like-to-book-a-flight.r8 file" << std::endl;
      return 1;
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);
  if (!file.read(buffer.data(), size)) {
      std::cerr << "Failed to read I-would-like-to-book-a-flight.r8 file"<< std::endl;
      return 1;
  }
  std::cout << "Read " << size << " bytes from I-would-like-to-book-a-flight.r8 file" << std::endl;

  // Init Lex client
  Aws::SDKOptions options;
  Aws::InitAPI(options);

  StartConversationRequest m_request;
  StartConversationHandler m_handler;

  Aws::String awsLocale("en_US");
  Aws::String sessionId(uuid_str);
  Aws::Client::ClientConfiguration config;
  config.region = "us-east-1";

  Aws::UniquePtr<LexRuntimeV2Client> m_client = Aws::MakeUnique<LexRuntimeV2Client>("lex_app", AWSCredentials(awsAccessKey, awsSecretAccessKey), config);

  size_t idx = 0;

  // configure handler
  m_handler.SetHeartbeatEventCallback([&](const HeartbeatEvent&)
  {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time);
    std::cout << "Received Lex Heartbeat after " <<  duration.count() << " ms" << std::endl;
  });

  m_handler.SetPlaybackInterruptionEventCallback([&](const PlaybackInterruptionEvent& ev){
    cJSON* json = lex2Json(ev);
    char* data = cJSON_AS4CPP_PrintUnformatted(json);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time);
    std::cout << "Received Lex SetPlaybackInterruptionEventCallback after " <<  duration.count() << " ms" << std::endl;
  });

  m_handler.SetTranscriptEventCallback([&](const TranscriptEvent& ev)
  {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time);
    cJSON* json = lex2Json(ev);
    char* data = cJSON_AS4CPP_PrintUnformatted(json);
    std::cout << "Received Lex Transcript: [" << data << "] after " <<  duration.count() << " ms" << std::endl;
  });

  m_handler.SetTextResponseEventCallback([&](const TextResponseEvent& ev){
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time);
		cJSON* json = lex2Json(ev);
    char* data = cJSON_AS4CPP_PrintUnformatted(json);
    std::cout << idx * 20 << "ms, Received Lex TextResponse: [" << data << "] after " <<  duration.count() << " ms" << std::endl;
  });

  m_handler.SetAudioResponseEventCallback([&](const AudioResponseEvent& ev){
    // std::cout << "Received Lex SetAudioResponseEventCallback:" << std::endl;
  });


  // config request
  m_request.SetBotId(botID);
  m_request.SetBotAliasId(botAliasID);
  m_request.SetSessionId(sessionId);
  m_request.SetEventStreamHandler(m_handler);
  m_request.SetLocaleId(awsLocale);

  m_client->StartConversationAsync(m_request,
  // OnRequest
  [&m_pStream](StartConversationRequestEventStream& stream){
    m_pStream = &stream;
    ConfigurationEvent configurationEvent;
    configurationEvent.SetResponseContentType("audio/mpeg");
    stream.WriteConfigurationEvent(configurationEvent);
    stream.flush();

    PlaybackCompletionEvent playbackCompletionEvent;
    stream.WritePlaybackCompletionEvent(playbackCompletionEvent);
    stream.flush();
  },
  //On response
  [&](const LexRuntimeV2Client* pClient,
            const StartConversationRequest& request,
            const StartConversationOutcome& outcome,
            const std::shared_ptr<const Aws::Client::AsyncCallerContext>&)
    {
      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time);
      std::cout << "Received Lex StartConversationOutcome: " << outcome.GetError() << " after " <<  duration.count() << " ms" << std::endl;
    }, nullptr/*context*/);

  int CHUNK_LENGTH = 320;
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<char> chunk(CHUNK_LENGTH);

  start_time = std::chrono::high_resolution_clock::now();
  while (idx < size) {
      size_t next_idx = std::min(idx + CHUNK_LENGTH, buffer.size());
      std::copy(buffer.begin() + idx, buffer.begin() + next_idx, chunk.begin());
      Aws::Utils::ByteBuffer audio((const unsigned char *)chunk.data(), next_idx - idx);

      AudioInputEvent audioInputEvent;
      audioInputEvent.SetAudioChunk(audio);
      audioInputEvent.SetContentType("audio/lpcm; sample-rate=8000; sample-size-bits=16; channel-count=1; is-big-endian=false");

      if (!m_pStream->WriteAudioInputEvent(audioInputEvent) || !m_pStream->flush()) {
          // Handle error
      }

      idx += CHUNK_LENGTH;
      
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> elapsed = end - start;
      if (elapsed.count() < 20) {
          std::this_thread::sleep_for(std::chrono::milliseconds(20) - elapsed);
      }
      start = std::chrono::high_resolution_clock::now();
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time);
  std::cout << "Shut down after " <<  duration.count() << " ms" << std::endl;

  //shutdown lex
  Aws::ShutdownAPI(options);
}

