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

#include "maidsafe/vault/maid_account_holder/maid_account_sync_handler.h"

#include "boost/filesystem/operations.hpp"

#include "maidsafe/common/error.h"

#include "maidsafe/vault/sync_pb.h"
#include "maidsafe/vault/utils.h"


namespace fs = boost::filesystem;

namespace maidsafe {

namespace vault {

MaidAccountSyncHandler::MaidAccountSyncHandler(const boost::filesystem::path& vault_root_dir)
    : kMaidAccountsSyncRoot_(vault_root_dir / "maids_sync"),
      maid_accounts_sync_() {
  fs::exists(kMaidAccountsSyncRoot_) || fs::create_directory(kMaidAccountsSyncRoot_);
}

nfs::Reply MaidAccountSyncHandler::HandleReceivedSyncInfo(
    const NonEmptyString& serialised_sync_info, const NodeId& source_id) {
  protobuf::SyncInfo sync_info;
  if (!sync_info.ParseFromString(serialised_sync_info.string())) {
    LOG(kError) << "Failed to parse reply";
    return nfs::Reply(CommonErrors::parsing_error);
  }

  MaidAccount::serialised_info_type serialised_account_info(
                                        NonEmptyString(sync_info.maid_account()));
  Accumulator<MaidName>::serialised_requests serialised_request(
                                        NonEmptyString(sync_info.accumulator_entries()));
  MaidName maid_name(Identity((sync_info.maid_name())));
  std::vector<boost::filesystem::path> required_file;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto itr = std::find_if(maid_accounts_sync_.begin(),
                            maid_accounts_sync_.end(),
                            [=](const std::unique_ptr<MaidAccountSync>& maid_accounts_sync) {
                                return (maid_accounts_sync->kMaidName() == maid_name);
                            });
    if (itr == maid_accounts_sync_.end()) { //  Sync for account not exists
      std::unique_ptr<MaidAccountSync> maid_account_sync(new MaidAccountSync(maid_name));
      required_file = maid_account_sync->AddSyncInfoUpdate(source_id, serialised_account_info,
                                                           serialised_request);
      maid_accounts_sync_.push_back(std::move(maid_account_sync));
    } else {
      required_file = (*itr)->AddSyncInfoUpdate(source_id, serialised_account_info, serialised_request);
    }
  }
  return nfs::Reply(CommonErrors::success);
}

nfs::Reply MaidAccountSyncHandler::HandleSyncArchiveFiles(const NonEmptyString& /*archive_files*/) {
  return nfs::Reply(CommonErrors::success);
}

}  // namespace vault

}  // namespace maidsafe
