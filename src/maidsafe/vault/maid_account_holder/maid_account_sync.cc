/***************************************************************************************************
 *  Copyright 2012 maidsafe.net limited                                                            *
 *                                                                                                 *
 *  The following source code is property of MaidSafe.net limited and is not meant for external    *
 *  use. The use of this code is governed by the licence file licence.txt found in the root of     *
 *  this directory and also on www.maidsafe.net.                                                   *
 *                                                                                                 *
 *  You are not free to copy, amend or otherwise use this source code without the explicit written *
 *  permission of the board of directors of MaidSafe.net.                                          *
 **************************************************************************************************/

#include "maidsafe/vault/maid_account_holder/maid_account_sync.h"

#include <string>

#include "boost/variant/apply_visitor.hpp"

#include "maidsafe/common/utils.h"

#include "maidsafe/vault/demultiplexer.h"
#include "maidsafe/vault/pmid_account_holder/pmid_account.h"
#include "maidsafe/vault/maid_account_holder/maid_account_pb.h"


namespace fs = boost::filesystem;

namespace maidsafe {

namespace vault {

std::vector<boost::filesystem::path> MaidAccountSync::AddSyncInfoUpdate(
    const NodeId& node_id,
    const MaidAccount::serialised_info_type& /*serialised_account_info*/,
    const Accumulator<passport::PublicMaid::name_type>::serialised_requests& serialised_request) {

  SyncInfoUpdate sync_update;
  sync_update.node_id = node_id;
//  auto account_info_and_file_names = MaidAccount::ParseAccountSyncInfo(serialised_account_info);
//  sync_update.account_info = account_info_and_file_names.first;
//  sync_update.shared_file_names = account_info_and_file_names.second;
  sync_update.handled_requests = Accumulator<passport::PublicMaid::name_type>::Parse(serialised_request);
  sync_update.requested_file_names = GetRequiredFileNames();
  sync_updates_.push_back(SyncInfoUpdate());
  return sync_update.requested_file_names;
}

std::vector<boost::filesystem::path> MaidAccountSync::GetRequiredFileNames() {
  // Check other requested/ available files and return missing bits required
  return std::vector<boost::filesystem::path>();
}

void MaidAccountSync::AddDownloadedFile(boost::filesystem::path /*file_name*/,
                                        const NonEmptyString& /*file_contents*/) {

}

bool MaidAccountSync::IsReadyForMerge() {
  // True when at least 3 updates available and all requested files are downloaded.
  return false;
}

bool MaidAccountSync::MergeSyncResults(std::unique_ptr<MaidAccount>& /*account*/,
                                       Accumulator<MaidName>& /*accumulator*/) {
  return true;
}

}  // namespace vault

}  // namespace maidsafe
