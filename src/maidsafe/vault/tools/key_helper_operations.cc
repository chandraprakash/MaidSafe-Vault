/***************************************************************************************************
 *  Copyright 2012 MaidSafe.net limited                                                            *
 *                                                                                                 *
 *  The following source code is property of MaidSafe.net limited and is not meant for external    *
 *  use.  The use of this code is governed by the licence file licence.txt found in the root of    *
 *  this directory and also on www.maidsafe.net.                                                   *
 *                                                                                                 *
 *  You are not free to copy, amend or otherwise use this source code without the explicit         *
 *  written permission of the board of directors of MaidSafe.net.                                  *
 **************************************************************************************************/

#include "maidsafe/vault/tools/key_helper_operations.h"

#include "boost/filesystem/operations.hpp"

#include "maidsafe/common/test.h"

#include "maidsafe/routing/parameters.h"

#include "maidsafe/nfs/client_utils.h"

#include "maidsafe/vault/tools/tools_exception.h"

namespace maidsafe {

namespace vault {

namespace tools {

namespace {

namespace fs = boost::filesystem;
namespace po = boost::program_options;

typedef boost::asio::ip::udp::endpoint UdpEndpoint;
typedef std::future<passport::PublicPmid> PublicPmidFuture;

nfs::Reply GetReply(const std::string& response) {
  return nfs::Reply(nfs::Reply::serialised_type(NonEmptyString(response)));
}

asymm::PublicKey GetPublicKeyFromReply(const passport::PublicPmid::name_type& name,
                                       const nfs::Reply& reply) {
  passport::PublicPmid pmid(name, passport::PublicPmid::serialised_type(reply.data()));
  return pmid.public_key();
}

std::mutex g_mutex;
std::condition_variable g_cond_var;
bool g_ctrlc_pressed;

void CtrlCHandler(int /*signum*/) {
//   LOG(kInfo) << " Signal received: " << signum;
  std::lock_guard<std::mutex> lock(g_mutex);
  g_ctrlc_pressed = true;
  g_cond_var.notify_one();
}

void CheckAllFutures(const PmidVector& all_pmids,
                     std::vector<PublicPmidFuture>& futures,
                     size_t& verified_keys) {
  auto equal_pmids = [] (const passport::PublicPmid& lhs, const passport::PublicPmid& rhs) {
                       return lhs.name() == rhs.name() &&
                              asymm::MatchingKeys(lhs.public_key(), rhs.public_key());
                     };
  for (size_t i(0); i != futures.size(); ++i) {
    try {
      if (equal_pmids(futures.at(i).get(), passport::PublicPmid(all_pmids.at(i))))
        ++verified_keys;
    }
    catch(...) {
      std::cout << "Failed getting key #" << i << std::endl;
    }
  }

}

}  // namespace

NetworkGenerator::NetworkGenerator() {}

void NetworkGenerator::SetupBootstrapNodes(const PmidVector& all_pmids) {
  std::cout << "Creating zero state routing network..." << std::endl;
  BootstrapData bootstrap_data(all_pmids.at(0), all_pmids.at(1));

  std::vector<passport::PublicPmid> all_public_pmids;
  all_public_pmids.reserve(all_pmids.size());
  for (auto& pmid : all_pmids)
    all_public_pmids.push_back(passport::PublicPmid(pmid));
  nfs::PublicKeyGetter public_key_getter(*bootstrap_data.routing1, all_public_pmids);

  routing::Functors functors1, functors2;
  functors1.request_public_key = functors2.request_public_key =
      [&public_key_getter, this] (NodeId node_id, const routing::GivePublicKeyFunctor& give_key) {
        DoOnPublicKeyRequested(node_id, give_key, public_key_getter);
      };

  endpoint1_ = UdpEndpoint(GetLocalIp(), test::GetRandomPort());
  endpoint2_ = UdpEndpoint(GetLocalIp(), test::GetRandomPort());
  auto a1 = std::async(std::launch::async,
                       [&, this] {
                         return bootstrap_data.routing1->ZeroStateJoin(functors1,
                                                                       endpoint1_,
                                                                       endpoint2_,
                                                                       bootstrap_data.info2);
                       });
  auto a2 = std::async(std::launch::async,
                       [&, this] {
                         return bootstrap_data.routing2->ZeroStateJoin(functors2,
                                                                       endpoint2_,
                                                                       endpoint1_,
                                                                       bootstrap_data.info1);
                       });
  if (a1.get() != 0 || a2.get() != 0) {
    LOG(kError) << "SetupNetwork - Could not start bootstrap nodes.";
    throw ToolsException("Failed to set up bootstrap nodes");
  }

  // just wait till process receives termination signal
  std::cout << "Bootstrap nodes are running, press Ctrl+C to terminate." << std::endl
            << "Endpoints: " << endpoint1_ << " and " << endpoint2_ << std::endl;
  signal(SIGINT, CtrlCHandler);
  std::unique_lock<std::mutex> lock(g_mutex);
  g_cond_var.wait(lock, [this] { return g_ctrlc_pressed; });  // NOLINT
  LOG(kInfo) << "Shutting down bootstrap nodes.";
}

std::vector<boost::asio::ip::udp::endpoint> NetworkGenerator::BootstrapEndpoints() const {
  std::vector<boost::asio::ip::udp::endpoint> endpoints(1, endpoint1_);
  endpoints.push_back(endpoint2_);
  return std::move(endpoints);
}

void NetworkGenerator::DoOnPublicKeyRequested(const NodeId& node_id,
                                              const routing::GivePublicKeyFunctor& give_key,
                                              nfs::PublicKeyGetter& public_key_getter) {
  passport::PublicPmid::name_type name(Identity(node_id.string()));
  public_key_getter.GetKey<passport::PublicPmid>(
      name,
      [name, give_key] (nfs::Reply reply) {
        if (reply.IsSuccess()) {
          try {
            asymm::PublicKey public_key(GetPublicKeyFromReply(name, reply));
            give_key(public_key);
          }
          catch(const std::exception& ex) {
            LOG(kError) << "Failed to get key for " << DebugId(name) << " : " << ex.what();
          }
        }
      });
}

ClientTester::ClientTester(const std::vector<UdpEndpoint>& peer_endpoints)
    : client_anmaid_(),
      client_maid_(client_anmaid_),
      client_pmid_(client_maid_),
      client_routing_(nullptr/*&client_maid_*/),
      functors_(),
      client_nfs_() {
  auto future(RoutingJoin(peer_endpoints));
  std::future_status status(future.wait_for(std::chrono::seconds(10)));
  if (status == std::future_status::timeout || !future.get())
    throw ToolsException("Failed to join client to network.");
  LOG(kInfo) << "Bootstrapped anonymous node to store keys";
  client_nfs_.reset(new nfs::ClientMaidNfs(client_routing_, client_maid_));
}

ClientTester::~ClientTester() {}

std::future<bool> ClientTester::RoutingJoin(const std::vector<UdpEndpoint>& peer_endpoints) {
  std::once_flag join_promise_set_flag;
  std::shared_ptr<std::promise<bool> > join_promise(std::make_shared<std::promise<bool> >());
  functors_.network_status = [&join_promise_set_flag, join_promise] (int result) {
                               std::cout << "Network health: " << result  << std::endl;
                               std::call_once(join_promise_set_flag,
                                              [join_promise, &result] {
                                                join_promise->set_value(result > -1);
                                              });
                             };
  client_routing_.Join(functors_, peer_endpoints);
  return std::move(join_promise->get_future());
}

KeyStorer::KeyStorer(const std::vector<UdpEndpoint>& peer_endpoints)
    : ClientTester(peer_endpoints) {}

void KeyStorer::Store(const PmidVector& all_pmids) {
  int stores(0);
  auto store_keys = [this, &stores] (const passport::Pmid& pmid, std::promise<bool>& promise) {
                      std::cout << "Storing #" << stores++ << std::endl;
                      passport::PublicPmid p_pmid(pmid);
                      nfs::Put<passport::PublicPmid>(*client_nfs_,
                                                     p_pmid,
                                                     client_pmid_.name(),
                                                     routing::Parameters::node_group_size,
                                                     callback(std::ref(promise)));
                    };

  std::vector<BoolPromise> bool_promises(all_pmids.size());
  std::vector<BoolFuture> bool_futures;
  size_t promises_index(0), error_stored_keys(0);
  for (auto& pmid : all_pmids) {  // store all PMIDs
    bool_futures.push_back(bool_promises.at(promises_index).get_future());
    std::async(std::launch::async, store_keys, pmid, std::ref(bool_promises.at(promises_index)));
    ++promises_index;
  }

  for (auto& future : bool_futures) {
    if (future.has_exception() || !future.get())
      ++error_stored_keys;
  }

  if (error_stored_keys > 0) {
    throw ToolsException(std::string("Could not store " + std::to_string(error_stored_keys) +
                                     " out of " + std::to_string(all_pmids.size())));
  }
  std::cout << "StoreKeys - Stored all " << all_pmids.size() << " keys." << std::endl;
}

std::function<void(nfs::Reply)> KeyStorer::callback(std::promise<bool>& promise) {
  return [&promise] (nfs::Reply reply) {
           try {
             promise.set_value(reply.IsSuccess());
           }
           catch(...) {
             promise.set_exception(std::current_exception());
           }
         };
}

KeyVerifier::KeyVerifier(const std::vector<UdpEndpoint>& peer_endpoints)
    : ClientTester(peer_endpoints) {}

void KeyVerifier::Verify(const PmidVector& all_pmids) {
  int gets(0);
  auto verify_keys = [this, &gets] (const passport::Pmid& pmid, PublicPmidFuture& future) {
                       std::cout << "Getting #" << gets++ << std::endl;
                       passport::PublicPmid p_pmid(pmid);
                       future = std::move(nfs::Get<passport::PublicPmid>(*client_nfs_,
                                                                         p_pmid.name()));
                     };
  std::vector<PublicPmidFuture> futures(all_pmids.size());
  size_t futures_index(0), verified_keys(0);
  for (auto& pmid : all_pmids) {
    std::async(std::launch::async, verify_keys, pmid, std::ref(futures.at(futures_index)));
    ++futures_index;
  }
  CheckAllFutures(all_pmids, futures, verified_keys);

  if (verified_keys < all_pmids.size()) {
    throw ToolsException(std::string("Not all keys were verified: " +
                                     std::to_string(verified_keys) + " out of " +
                                     std::to_string(all_pmids.size())));
  }
  std::cout << "VerifyKeys - Verified all " << verified_keys << " keys." << std::endl;
}

DataChunkStorer::DataChunkStorer(const std::vector<UdpEndpoint>& peer_endpoints)
    : ClientTester(peer_endpoints),
      run_(false) {}

void DataChunkStorer::StopTest() { run_ = false; }

void DataChunkStorer::Test(int32_t quantity) {
  int32_t rounds(0);
  size_t num_chunks(0), num_store(0), num_get(0);
  while (!Done(quantity, rounds))
    OneChunkRun(num_chunks, num_store, num_get);
  if (num_chunks != num_get)
    throw ToolsException("Failed to store and verify all data chunks.");
}

bool DataChunkStorer::Done(int32_t quantity, int32_t rounds) const {
  return quantity < 1 ? run_.load() : rounds < quantity;
}

void DataChunkStorer::OneChunkRun(size_t& num_chunks, size_t& num_store, size_t& num_get) {
  ImmutableData::serialised_type content(NonEmptyString(RandomString(1 << 18)));  // 256 KB
  ImmutableData::name_type name(Identity(crypto::Hash<crypto::SHA512>(content.data)));
  ImmutableData chunk_data(name, content);
  ++num_chunks;

  if (StoreOneChunk(chunk_data)) {
    LOG(kInfo) << "Stored chunk " << HexSubstr(name.data) << std::endl;
    ++num_store;
  } else {
    LOG(kError) << "Failed to store chunk " << HexSubstr(name.data);
    return;
  }

  // The current client is anonymous, which incurs a 10 mins faded out for stored data
  LOG(kInfo) << "Going to retrieve the stored chunk";
  if (GetOneChunk(chunk_data)) {
    LOG(kInfo) << "Got chunk " << HexSubstr(name.data);
    ++num_get;
  } else {
    LOG(kError) << "Failed to store chunk " << HexSubstr(name.data);
  }
}

bool DataChunkStorer::StoreOneChunk(const ImmutableData& chunk_data) {
  std::promise<bool> store_promise;
  std::future<bool> store_future(store_promise.get_future());
  routing::ResponseFunctor cb([&store_promise] (std::string response) {
                                          try {
                                            nfs::Reply reply(GetReply(response));
                                            store_promise.set_value(reply.IsSuccess());
                                          }
                                          catch(...) {
                                            store_promise.set_exception(std::current_exception());
                                          }
                                        });
  LOG(kInfo) << "Storing chunk " << HexSubstr(chunk_data.data()) << " ...";
  client_nfs_->Put(chunk_data, client_pmid_.name(), cb);
  return store_future.get();
}

bool DataChunkStorer::GetOneChunk(const ImmutableData& chunk_data) {
  std::promise<bool> get_promise;
  std::future<bool> get_future(get_promise.get_future());
  auto equal_immutables = [] (const ImmutableData& lhs, const ImmutableData& rhs) {
                            return lhs.name().data.string() == lhs.name().data.string() &&
                                   lhs.data().string() == rhs.data().string();
                          };

  routing::ResponseFunctor cb([&chunk_data, &get_promise, &equal_immutables]
                              (std::string response) {
                                try {
                                  nfs::Reply reply(GetReply(response));
                                  ImmutableData data(reply.data());
                                  get_promise.set_value(equal_immutables(chunk_data, data));
                                }
                                catch(...) {
                                  get_promise.set_exception(std::current_exception());
                                }
                              });
  client_nfs_->Get<ImmutableData>(chunk_data.name(), cb);
  return get_future.get();
}

// bool ExtendedTest(const size_t& chunk_set_count,
//                   const FobPairVector& all_keys,
//                   const std::vector<UdpEndpoint>& peer_endpoints) {
//   const size_t kNumClients(3);
//   assert(all_keys.size() >= 2 + kNumClients);
//   std::vector<std::unique_ptr<pd::Node>> clients;
//   std::vector<test::TestPath> client_paths;
//   std::vector<std::unique_ptr<pcs::RemoteChunkStore>> rcs;
//   for (size_t i = 2; i < 2 + kNumClients; ++i) {
//     std::cout << "Setting up client " << (i - 1) << " ..." << std::endl;
//     std::unique_ptr<pd::Node> client(new pd::Node);
//     client_paths.push_back(test::CreateTestPath("MaidSafe_Test_KeysHelper"));
//     client->set_account_name(all_keys[i].first.identity);
//     client->set_fob(all_keys[i].first);
//     if (client->Start(*client_paths.back(), peer_endpoints) != 0) {
//       std::cout << "Failed to start client " << (i - 1) << std::endl;
//       return false;
//     }
//     rcs.push_back(std::move(std::unique_ptr<pcs::RemoteChunkStore>(new pcs::RemoteChunkStore(
//         client->chunk_store(), client->chunk_manager(), client->chunk_action_authority()))));
//     clients.push_back(std::move(client));
//   }
//
//   const size_t kNumChunks(3 * chunk_set_count);
//   std::atomic<size_t> total_ops(0), succeeded_ops(0);
//   typedef std::map<pd::ChunkName,
//                    std::pair<NonEmptyString, Fob>> ChunkMap;
//   ChunkMap chunks;
//   for (size_t i = 0; i < kNumChunks; ++i) {
//     pd::ChunkName chunk_name;
//     NonEmptyString chunk_contents;
//     auto chunk_fob = clients[0]->fob();
//     switch (i % 3) {
//       case 0:  // DEF
//         chunk_contents = NonEmptyString(RandomString(1 << 18));  // 256 KB
//         chunk_name = priv::ApplyTypeToName(
//             crypto::Hash<crypto::SHA512>(chunk_contents),
//             priv::ChunkType::kDefault);
//         break;
//       case 1:  // MBO
//         pd::CreateSignedDataPayload(
//             NonEmptyString(RandomString(1 << 16)),  // 64 KB
//             chunk_fob.keys.private_key,
//             chunk_contents);
//         chunk_name = priv::ApplyTypeToName(
//                          NodeId(NodeId::kRandomId),
//                          priv::ChunkType::kModifiableByOwner);
//         break;
//       case 2:  // SIG
//         chunk_fob = pd::GenerateIdentity();
//         pd::CreateSignaturePacket(chunk_fob, chunk_name, chunk_contents);
//         break;
//     }
//     chunks[chunk_name] = std::make_pair(chunk_contents, chunk_fob);
//     LOG(kInfo) << "ExtendedTest - Generated chunk "
//                << pd::DebugChunkName(chunk_name) << " of size "
//                << BytesToBinarySiUnits(chunk_contents.string().size());
//   }
//
//   enum class RcsOp {
//     kStore = 0,
//     kModify,
//     kDelete,
//     kGet
//   };
//
//   auto do_rcs_op = [&rcs, &total_ops, &succeeded_ops](RcsOp rcs_op,
//                                                       ChunkMap::value_type& chunk,
//                                                       bool expect_success) {
//     std::mutex mutex;
//     std::condition_variable cond_var;
//     std::atomic<bool> cb_done(false), cb_result(false);
//     auto cb = [&cond_var, &cb_done, &cb_result](bool result) {
//       cb_result = result;
//       cb_done = true;
//       cond_var.notify_one();
//     };
//     bool result(false);
//     std::string action_name;
//     switch (rcs_op) {
//       case RcsOp::kStore:
//         action_name = "Storing";
//         result = rcs[0]->Store(chunk.first, chunk.second.first, cb, chunk.second.second);
//         break;
//       case RcsOp::kModify:
//         action_name = "Modifying";
//         {
//           NonEmptyString new_content;
//           pd::CreateSignedDataPayload(
//               NonEmptyString(RandomString(1 << 16)),  // 64 KB
//               chunk.second.second.keys.private_key,
//               new_content);
//           if (expect_success)
//             chunk.second.first = new_content;
//           result = rcs[0]->Modify(chunk.first, chunk.second.first, cb, chunk.second.second);
//         }
//         break;
//       case RcsOp::kDelete:
//         action_name = "Deleting";
//         result = rcs[0]->Delete(chunk.first, cb, chunk.second.second);
//         break;
//       case RcsOp::kGet:
//         action_name = "Getting";
//         result = (chunk.second.first.string() == rcs[1]->Get(chunk.first, chunk.second.second,
//                                                              false));
//         break;
//     }
//     if (rcs_op != RcsOp::kGet && result) {
//       std::unique_lock<std::mutex> lock(mutex);
//       cond_var.wait(lock, [&cb_done] { return cb_done.load(); });  // NOLINT
//       result = cb_result;
//     }
//     ++total_ops;
//     if (result == expect_success) {
//       ++succeeded_ops;
//       LOG(kSuccess) << "ExtendedTest - " << action_name << " "
//                     << pd::DebugChunkName(chunk.first) << " "
//                     << (result ? "succeeded" : "failed") << " as expected.";
//     } else {
//       LOG(kError) << "ExtendedTest - " << action_name << " "
//                   << pd::DebugChunkName(chunk.first) << " "
//                   << (result ? "succeeded" : "failed") << " unexpectedly.";
//     }
//   };
//
//   size_t prev_ops_diff(0);
//   auto const kStageDescriptions = []()->std::vector<std::string> {
//     std::vector<std::string> desc;
//     desc.push_back("storing chunks");
//     desc.push_back("retrieving chunks");
//     desc.push_back("modifying chunks");
//     desc.push_back("retrieving modified chunks");
//     desc.push_back("deleting chunks");
//     desc.push_back("retrieving deleted chunks");
//     desc.push_back("modifying non-existing chunks");
//     desc.push_back("re-storing chunks");
//     desc.push_back("retrieving chunks");
//     desc.push_back("deleting chunks");
//     return desc;
//   }();
//   for (int stage = 0; stage < 9; ++stage) {
//     std::cout << "Processing test stage " << (stage + 1) << ": " << kStageDescriptions[stage]
//               << "..." << std::endl;
//     for (auto& rcs_it : rcs)
//       rcs_it->Clear();
//     auto start_time = boost::posix_time::microsec_clock::local_time();
//     for (auto& chunk : chunks) {
//       auto chunk_type = priv::GetChunkType(chunk.first);
//       switch (stage) {
//         case 0:  // store all chunks
//           do_rcs_op(RcsOp::kStore, chunk, true);
//           break;
//         case 1:  // get all chunks to verify storage
//           do_rcs_op(RcsOp::kGet, chunk, true);
//           break;
//         case 2:  // modify all chunks, should only succeed for MBO
//           do_rcs_op(RcsOp::kModify, chunk,
//                     chunk_type == priv::ChunkType::kModifiableByOwner);
//           break;
//         case 3:  // get all chunks to verify (non-)modification
//           do_rcs_op(RcsOp::kGet, chunk, true);
//           break;
//         case 4:  // delete all chunks
//           do_rcs_op(RcsOp::kDelete, chunk, true);
//           break;
//         case 5:  // get all chunks to verify deletion
//           do_rcs_op(RcsOp::kGet, chunk, false);
//           break;
//         case 6:  // modify all (non-existing) chunks
//           do_rcs_op(RcsOp::kModify, chunk, false);
//           break;
//         case 7:  // store all chunks again, only SIG should fail due to revokation
//           do_rcs_op(RcsOp::kStore, chunk,
//                     chunk_type != priv::ChunkType::kSignaturePacket);
//           break;
//         case 8:  // get all chunks again, only SIG should fail due to revokation
//           do_rcs_op(RcsOp::kGet,
//                     chunk,
//                     chunk_type != priv::ChunkType::kSignaturePacket);
//           break;
//         case 9:  // delete all chunks to clean up
//           do_rcs_op(RcsOp::kDelete, chunk, true);
//           break;
//         default:
//           break;
//       }
//     }
//     for (auto& rcs_it : rcs)
//       rcs_it->WaitForCompletion();
//     auto end_time = boost::posix_time::microsec_clock::local_time();
//     size_t ops_diff = total_ops - succeeded_ops;
//     std::cout << "Stage " << (stage + 1) << ": " << kStageDescriptions[stage] << " - "
//               << (ops_diff == prev_ops_diff ? "SUCCEEDED" : "FAILED") << " ("
//               << (end_time - start_time) << ")" << std::endl;
//     prev_ops_diff = ops_diff;
//   }
//
//   for (auto& client : clients)
//     client->Stop();
//
//   std::cout << "Extended test completed " << succeeded_ops << " of " << total_ops
//             << " operations for " << kNumChunks << " chunks successfully." << std::endl;
//   return succeeded_ops == total_ops;
// }

}  // namespace tools

}  // namespace vault

}  // namespace maidsafe
