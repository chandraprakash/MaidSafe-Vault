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

MaidAccountSync::MaidAccountSync(const MaidName& maid_name)
    : mutex_(),
      kMaidName_(maid_name),
      downloaded_files_(),
      sync_updates_(),
      is_ready_for_merge_(false) {}

MaidAccountSync::SyncInfoUpdate::SyncInfoUpdate(const NodeId& source_id_in,
    const MaidAccount::AccountInfo& account_info_in,
    const std::vector<Accumulator<passport::PublicMaid::name_type>::HandledRequest>
        handled_requests_in,
    const DiskBasedStorage::FileIdentities shared_file_ids_in,
    const DiskBasedStorage::FileIdentities requested_file_ids_in)
    : source_id(source_id_in),
      account_info(account_info_in),
      handled_requests(handled_requests_in),
      shared_file_ids(shared_file_ids_in),
      requested_file_ids(requested_file_ids_in) {}

MaidAccountSync::SyncInfoUpdate::SyncInfoUpdate(const SyncInfoUpdate& other)
    : source_id(other.source_id),
      account_info(other.account_info),
      handled_requests(other.handled_requests),
      shared_file_ids(other.shared_file_ids),
      requested_file_ids(other.requested_file_ids) {}

MaidAccountSync::SyncInfoUpdate& MaidAccountSync::SyncInfoUpdate::operator=(
    const SyncInfoUpdate& other) {
  source_id = other.source_id;
  account_info = other.account_info;
  handled_requests = other.handled_requests;
  shared_file_ids = other.shared_file_ids;
  requested_file_ids = other.requested_file_ids;
  return *this;
}

MaidAccountSync::SyncInfoUpdate::SyncInfoUpdate(SyncInfoUpdate&& other)
    : source_id(std::move(other.source_id)),
      account_info(std::move(other.account_info)),
      handled_requests(std::move(other.handled_requests)),
      shared_file_ids(std::move(other.shared_file_ids)),
      requested_file_ids(std::move(other.requested_file_ids)) {}

MaidAccountSync::SyncInfoUpdate&
MaidAccountSync::SyncInfoUpdate::operator=(SyncInfoUpdate&& other) {
  source_id = std::move(other.source_id);
  account_info = std::move(other.account_info);
  handled_requests = std::move(other.handled_requests);
  shared_file_ids = std::move(other.shared_file_ids);
  requested_file_ids = std::move(other.requested_file_ids);
  return *this;
}

DiskBasedStorage::FileIdentities MaidAccountSync::AddSyncInfoUpdate(
    const NodeId& source_id,
    const MaidAccount::serialised_info_type& serialised_account_info,
    const Accumulator<passport::PublicMaid::name_type>::serialised_requests& serialised_request) {

  auto account_info_and_file_ids = ParseMaidAccountSyncInfo(serialised_account_info);
  sync_updates_.push_back(SyncInfoUpdate(source_id,
                                         account_info_and_file_ids.first,
                                         ParseHandledRequest<MaidName>(serialised_request),
                                         account_info_and_file_ids.second,
                                         GetRequiredFileNames()));
  return sync_updates_.back().requested_file_ids;
}

 DiskBasedStorage::FileIdentities GetFileRequests(const NodeId& /*source_id*/) {
   return DiskBasedStorage::FileIdentities();
 }

DiskBasedStorage::FileIdentities MaidAccountSync::GetRequiredFileNames() {
  // Check other requested/ available files and return missing bits required
  return DiskBasedStorage::FileIdentities();
}

void MaidAccountSync::AddDownloadedFile(DiskBasedStorage::FileIdentity /*file_name*/,
                                        const NonEmptyString& /*file_contents*/) {
}

bool MaidAccountSync::IsReadyForMerge() {
  // True when at least 3 updates available and all requested files are downloaded.
  return false;
}

bool MaidAccountSync::MergeSyncResults(std::unique_ptr<MaidAccount>& /*account*/,
                                       Accumulator<MaidName>& /*accumulator*/) {

// Merge AccountInfo
  std::vector<PmidTotals> merged_pmid_totals_;







  return true;
}

}  // namespace vault

}  // namespace maidsafe
