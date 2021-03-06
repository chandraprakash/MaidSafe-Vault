/* Copyright 2013 MaidSafe.net limited

This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or later,
and The General Public License (GPL), version 3. By contributing code to this project You agree to
the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the root directory
of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available at:

http://www.novinet.com/license

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing permissions and limitations under the
License.
*/

#ifndef MAIDSAFE_VAULT_STRUCTURED_DATA_MANAGER_STRUCTURED_DATA_MANAGER_SERVICE_H_
#define MAIDSAFE_VAULT_STRUCTURED_DATA_MANAGER_STRUCTURED_DATA_MANAGER_SERVICE_H_

#include <mutex>
#include <string>
#include <type_traits>
#include <vector>

#include "maidsafe/common/types.h"
#include "maidsafe/passport/types.h"
#include "maidsafe/routing/routing_api.h"
#include "maidsafe/nfs/message.h"
#include "maidsafe/nfs/types.h"
#include "maidsafe/nfs/persona_id.h"
#include "maidsafe/vault/accumulator.h"
#include "maidsafe/vault/db.h"
#include "maidsafe/vault/sync.h"
#include "maidsafe/vault/sync.pb.h"
#include "maidsafe/vault/types.h"
#include "maidsafe/vault/structured_data_manager/structured_data_manager.h"
#include "maidsafe/vault/structured_data_manager/structured_data_merge_policy.h"
#include "maidsafe/vault/manager_db.h"


namespace maidsafe {

namespace vault {

class StructuredDataManagerService {
 public:
  typedef Identity StructuredDataAccountName;
  StructuredDataManagerService(const passport::Pmid& pmid,
                               routing::Routing& routing,
                               const boost::filesystem::path& path);
  template<typename Data>
  void HandleMessage(const nfs::Message& message,
                     const routing::ReplyFunctor& reply_functor);
  void HandleChurnEvent(routing::MatrixChange matrix_change);

 private:
  StructuredDataManagerService(const StructuredDataManagerService&);
  StructuredDataManagerService& operator=(const StructuredDataManagerService&);
  StructuredDataManagerService(StructuredDataManagerService&&);
  StructuredDataManagerService& operator=(StructuredDataManagerService&&);

  void ValidateClientSender(const nfs::Message& message) const;
  void ValidateSyncSender(const nfs::Message& message) const;
  std::vector<StructuredDataVersions::VersionName>
                       GetVersionsFromMessage(const nfs::Message& msg) const;
  NonEmptyString GetSerialisedRecord(const StructuredDataManager::DbKey& db_key);
  //// =============== Get data ====================================================================
  void HandleGet(const nfs::Message& message, routing::ReplyFunctor reply_functor);
  void HandleGetBranch(const nfs::Message& message, routing::ReplyFunctor reply_functor);

  //// =============== Sync ========================================================================
  template<typename Data>
  void Synchronise(const nfs::Message& message);
  void HandleSynchronise(const nfs::Message& message);

  //// =============== Churn ============================================================
  void HandleAccountTransfer(const nfs::Message& message);

  routing::Routing& routing_;
  std::mutex accumulator_mutex_;
  std::mutex sync_mutex_;
  Accumulator<StructuredDataAccountName> accumulator_;
  ManagerDb<StructuredDataManager> structured_data_db_;
  const NodeId kThisNodeId_;
  Sync<StructuredDataMergePolicy> sync_;
  StructuredDataManagerNfs nfs_;
};

}  // namespace vault

}  // namespace maidsafe

#include "maidsafe/vault/structured_data_manager/structured_data_manager_service-inl.h"

#endif  // MAIDSAFE_VAULT_STRUCTURED_DATA_MANAGER_STRUCTURED_DATA_MANAGER_SERVICE_H_
