/*  Copyright 2012 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

#include "maidsafe/common/test.h"
#include "maidsafe/passport/passport.h"
#include "maidsafe/nfs/types.h"

#include "maidsafe/vault/unresolved_action.h"
#include "maidsafe/vault/unresolved_action.pb.h"
#include "maidsafe/vault/pmid_manager/service.h"
#include "maidsafe/vault/tests/tests_utils.h"

namespace maidsafe {

namespace vault {

namespace test {

//class PmidManagerServiceTest : public testing::Test {
// public:
//  PmidManagerServiceTest()
//      : pmid_(passport::CreatePmidAndSigner().first),
//        kTestRoot_(maidsafe::test::CreateTestPath("MaidSafe_Test_Vault")),
//        vault_root_dir_(*kTestRoot_),
//        routing_(pmid_),
//        pmid_manager_service_(pmid_, routing_, vault_root_dir_) {}

//  template <typename UnresolvedActionType>
//  std::vector<std::unique_ptr<UnresolvedActionType>> GetUnresolvedActions();

//  void AddGroup(const PmidManager::GroupName& group_name, const PmidManager::Metadata& metadata) {
//    pmid_manager_service_.group_db_.AddGroup(group_name, metadata);
//  }

//  PmidManager::Metadata GetMetadata(const PmidManager::GroupName& group_name) {
//    return pmid_manager_service_.group_db_.GetMetadata(group_name);
//  }

//  PmidManager::Value GetValue(const PmidManager::Key& key) {
//    return pmid_manager_service_.group_db_.GetValue(key);
//  }

//  template <typename ActionType>
//  void Commit(const PmidManager::Key& key, const ActionType& action) {
//    pmid_manager_service_.group_db_.Commit(key, action);
//  }

//  template <typename UnresolvedActionType>
//  void SendSync(const std::vector<UnresolvedActionType>& unresolved_actions,
//                const std::vector<routing::GroupSource>& group_source);

// protected:
//  passport::Pmid pmid_;
//  const maidsafe::test::TestPath kTestRoot_;
//  boost::filesystem::path vault_root_dir_;
//  routing::Routing routing_;
//  PmidManagerService pmid_manager_service_;
//};

//template <typename UnresolvedActionType>
//void PmidManagerServiceTest::SendSync(
//    const std::vector<UnresolvedActionType>& /*unresolved_actions*/,
//    const std::vector<routing::GroupSource>& /*group_source*/) {
//  UnresolvedActionType::No_genereic_handler_is_available__Specialisation_is_required;
//}

//template <>
//void PmidManagerServiceTest::SendSync<PmidManager::UnresolvedPut>(
//    const std::vector<PmidManager::UnresolvedPut>& unresolved_actions,
//    const std::vector<routing::GroupSource>& group_source) {
//  AddLocalActionAndSendGroupActions<PmidManagerService, PmidManager::UnresolvedPut,
//                                    SynchroniseFromPmidManagerToPmidManager>(
//      &pmid_manager_service_, pmid_manager_service_.sync_puts_, unresolved_actions, group_source);
//}

//template <>
//void PmidManagerServiceTest::SendSync<PmidManager::UnresolvedDelete>(
//    const std::vector<PmidManager::UnresolvedDelete>& unresolved_actions,
//    const std::vector<routing::GroupSource>& group_source) {
//  AddLocalActionAndSendGroupActions<PmidManagerService, PmidManager::UnresolvedDelete,
//                                    SynchroniseFromPmidManagerToPmidManager>(
//      &pmid_manager_service_, pmid_manager_service_.sync_deletes_, unresolved_actions,
//      group_source);
//}

//template <typename UnresolvedActionType>
//std::vector<std::unique_ptr<UnresolvedActionType>> PmidManagerServiceTest::GetUnresolvedActions() {
//  UnresolvedActionType::No_genereic_handler_is_available__Specialisation_is_required;
//  return std::vector<std::unique_ptr<UnresolvedActionType>>();
//}

//template <>
//std::vector<std::unique_ptr<PmidManager::UnresolvedPut>>
//PmidManagerServiceTest::GetUnresolvedActions<PmidManager::UnresolvedPut>() {
//  return pmid_manager_service_.sync_puts_.GetUnresolvedActions();
//}

//template <>
//std::vector<std::unique_ptr<PmidManager::UnresolvedDelete>>
//PmidManagerServiceTest::GetUnresolvedActions<PmidManager::UnresolvedDelete>() {
//  return pmid_manager_service_.sync_deletes_.GetUnresolvedActions();
//}

//TEST_F(PmidManagerServiceTest, BEH_VariousRequests) {
//  //  PutRequestFromDataManagerToPmidManager
//  {
//    auto content(CreateContent<PutRequestFromDataManagerToPmidManager::Contents>());
//    auto put_request(CreateMessage<PutRequestFromDataManagerToPmidManager>(content));
//    auto group_source(CreateGroupSource(NodeId(put_request.contents->name.raw_name.string())));
//    EXPECT_NO_THROW(GroupSendToGroup(&pmid_manager_service_, put_request, group_source,
//                                     routing::GroupId(this->routing_.kNodeId())));
//    EXPECT_TRUE(GetUnresolvedActions<PmidManager::UnresolvedPut>().size() == 0);
//  }

//  //  BEH_PutFailureFromPmidNodeToPmidManager)
//  {
//    PmidManagerMetadata metadata(PmidName(pmid_.name()));
//    metadata.claimed_available_size = kTestChunkSize * 100;
//    AddGroup(PmidName(pmid_.name()), PmidManagerMetadata());
//    auto content(CreateContent<PutFailureFromPmidNodeToPmidManager::Contents>());
//    auto put_failure(CreateMessage<PutFailureFromPmidNodeToPmidManager>(content));
//    EXPECT_NO_THROW(SingleSendsToGroup(&pmid_manager_service_, put_failure,
//                                       routing::SingleSource(NodeId(NodeId::IdType::kRandomId)),
//                                       routing::GroupId(NodeId(pmid_.name()->string()))));
//    EXPECT_TRUE(GetUnresolvedActions<PmidManager::UnresolvedDelete>().size() == 0);
//    metadata = GetMetadata(PmidName(pmid_.name()));
//    EXPECT_TRUE(metadata.claimed_available_size == 0);
//  }

//  //  BEH_DeleteRequestFromDataManagerToPmidManager)
//  {
//    auto content(CreateContent<DeleteRequestFromDataManagerToPmidManager::Contents>());
//    auto delete_request(CreateMessage<DeleteRequestFromDataManagerToPmidManager>(content));
//    auto group_source(CreateGroupSource(NodeId(content.raw_name.string())));
//    EXPECT_NO_THROW(GroupSendToGroup(&pmid_manager_service_, delete_request, group_source,
//                                     routing::GroupId(NodeId(pmid_.name()->string()))));
//    EXPECT_TRUE(GetUnresolvedActions<PmidManager::UnresolvedDelete>().size() == 0);
//  }

//  //  BEH_GetPmidAccountRequestFromPmidNodeToPmidManager
//  {
//    auto content(CreateContent<GetPmidAccountRequestFromPmidNodeToPmidManager::Contents>());
//    auto get_pmid_account_request(
//        CreateMessage<GetPmidAccountRequestFromPmidNodeToPmidManager>(content));
//    EXPECT_NO_THROW(SingleSendsToGroup(&pmid_manager_service_, get_pmid_account_request,
//                                       routing::SingleSource(NodeId(pmid_.name()->string())),
//                                       routing::GroupId(NodeId(pmid_.name()->string()))));
//  }
//  {
//    auto content(CreateContent<PmidHealthRequestFromMaidManagerToPmidManager::Contents>());
//    auto get_pmid_account_request(
//        CreateMessage<PmidHealthRequestFromMaidManagerToPmidManager>(content));
//    NodeId maid_node(NodeId::IdType::kRandomId);
//    auto group_source(CreateGroupSource(maid_node));
//  }
//}


//TEST_F(PmidManagerServiceTest, BEH_PutThenDelete) {
//  ImmutableData data(NonEmptyString(RandomString(kTestChunkSize)));
//  auto group_source(CreateGroupSource(NodeId(pmid_.name()->string())));
//  PmidManager::Key key(PmidName(pmid_.name()), data.name(), ImmutableData::Tag::kValue);
//  AddGroup(PmidName(pmid_.name()), PmidManagerMetadata());

//  {  // Put
//    ActionPmidManagerPut action_put(kTestChunkSize, nfs::MessageId(RandomInt32()));
//    auto group_unresolved_action(
//        CreateGroupUnresolvedAction<PmidManager::UnresolvedPut>(key, action_put, group_source));
//    SendSync<PmidManager::UnresolvedPut>(group_unresolved_action, group_source);
//    auto value(GetValue(key));
//    auto metadata(GetMetadata(PmidName(pmid_.name())));
//    EXPECT_TRUE(value.size() == kTestChunkSize);
//    EXPECT_TRUE(metadata.stored_total_size == kTestChunkSize);
//    EXPECT_TRUE(metadata.stored_count == 1);
//  }
//  {  //  Delete
//    ActionPmidManagerDelete action_delete(false, false);
//    EXPECT_NO_THROW(GetMetadata(PmidName(pmid_.name()))) << "pmid name should be here";
//    auto group_unresolved_action(CreateGroupUnresolvedAction<PmidManager::UnresolvedDelete>(
//        key, action_delete, group_source));
//    SendSync<PmidManager::UnresolvedDelete>(group_unresolved_action, group_source);
//    EXPECT_ANY_THROW(GetValue(key)) << " this key shoud have been deleted";
//    EXPECT_THROW(GetMetadata(PmidName(pmid_.name())), std::exception)
//        << " Added one pmid and deleted it so account should be removed";
//  }
//}

}  //  namespace test

}  //  namespace vault

}  //  namespace maidsafe
