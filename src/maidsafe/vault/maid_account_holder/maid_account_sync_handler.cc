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

#include "maidsafe/vault/sync.pb.h"
#include "maidsafe/vault/utils.h"


namespace fs = boost::filesystem;

namespace maidsafe {

namespace vault {

MaidAccountSyncHandler::MaidAccountSyncHandler(const boost::filesystem::path& vault_root_dir)
    : kMaidAccountsSyncRoot_(vault_root_dir / "maids_sync"),
      maid_accounts_sync_() {
  fs::exists(kMaidAccountsSyncRoot_) || fs::create_directory(kMaidAccountsSyncRoot_);
}

nfs::Reply MaidAccountSyncHandler::HandlePeriodicSync(
    const NonEmptyString& serialised_periodic_sync, const NodeId& /*source_id*/) {
  protobuf::PeriodicSync periodic_sync;
  if (!periodic_sync.ParseFromString(serialised_periodic_sync.string())) {
    LOG(kError) << "Failed to parse PeriodicSync";
    return nfs::Reply(CommonErrors::parsing_error);
  }
//  try {
//    MaidAccount::serialised_info_type serialised_account_info(
//        NonEmptyString(sync_info.maid_account()));
//    Accumulator<MaidName>::serialised_requests serialised_request(
//        NonEmptyString(sync_info.accumulator_entries()));
//    MaidName maid_name(Identity((sync_info.maid_name())));
//    DiskBasedStorage::FileIdentities required_files;
//    {
//      std::lock_guard<std::mutex> lock(mutex_);
//      auto itr = detail::FindAccount(maid_accounts_sync_, maid_name);
//      if (itr == maid_accounts_sync_.end()) { //  Sync for account not exists
//        std::unique_ptr<MaidAccountSync> maid_account_sync(new MaidAccountSync(maid_name));
//        required_files = maid_account_sync->AddSyncInfoUpdate(source_id, serialised_account_info,
//                                                              serialised_request);
//        maid_accounts_sync_.push_back(std::move(maid_account_sync));
//      } else {
//        required_files = (*itr)->AddSyncInfoUpdate(source_id, serialised_account_info,
//                                                   serialised_request);
//      }
//    }
//    return nfs::Reply(CommonErrors::success, SerilaiseFilesRequest(required_files));
//  } catch (const std::exception& /*ex*/) {
//    return nfs::Reply(CommonErrors::unknown); // TODO add error for this type ?
//  }
  return nfs::Reply(CommonErrors::unknown);
}

nfs::Reply MaidAccountSyncHandler::HandleAccountTransfer(const NonEmptyString& /*serialised_sync_info*/,
                                                         const NodeId& /*source_id*/) {
  return nfs::Reply(CommonErrors::unknown);
}

nfs::Reply MaidAccountSyncHandler::HandleArchiveFiles(const NonEmptyString& serialised_archive_files,
                                                      const NodeId& /*source_id*/) {
  protobuf::ArchiveFiles archive_files;
  if (!archive_files.ParseFromString(serialised_archive_files.string())) {
    LOG(kError) << "Failed to parse ArchiveFiles";
    // TODO request files from other nodes, Notify sync object about this failure
    return nfs::Reply(CommonErrors::parsing_error);
  }
//  try {
//    MaidName maid_name(Identity((archive_files.maid_name())));
//    DiskBasedStorage::FileIdentities required_files;
//    {
//      std::lock_guard<std::mutex> lock(mutex_);
//      auto itr = detail::FindAccount(maid_accounts_sync_, maid_name);
//      if (itr == maid_accounts_sync_.end())
//        return nfs::Reply(CommonErrors::unknown);  // TODO add error for this type ?

//      for (auto &i: sync_archive_files.files()) {
//        (*itr)->AddDownloadedFile(
//          DiskBasedStorage::FileIdentity((std::make_pair(i.file_id().index(), i.file_id().hash()))),
//          NonEmptyString(i.contents()));
//      }
//      required_files = (*itr)->GetFileRequests(source_id);
//    }
//    return nfs::Reply(CommonErrors::success, SerilaiseFilesRequest(required_files));
//  } catch (const std::exception& /*ex*/) {
//    return nfs::Reply(CommonErrors::unknown); // TODO add error for this type ?
//  }
  return nfs::Reply(CommonErrors::unknown);
}

//NonEmptyString MaidAccountSyncHandler::SerilaiseFilesRequest(
//    const DiskBasedStorage::FileIdentities& file_ids) {
//  protobuf::GetArchiveFiles get_archive_files;
//  for (auto& required_file : file_ids) {
//    auto file_id(get_archive_files.add_file_id());
//    file_id->set_index(required_file.first);
//    file_id->set_hash(required_file.second.string());
//  }
//  return NonEmptyString(get_archive_files.SerializeAsString());
//}

}  // namespace vault

}  // namespace maidsafe
