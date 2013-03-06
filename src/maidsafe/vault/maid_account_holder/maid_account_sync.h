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

#ifndef MAIDSAFE_VAULT_MAID_ACCOUNT_HOLDER_MAID_ACCOUNT_SYNC_H_
#define MAIDSAFE_VAULT_MAID_ACCOUNT_HOLDER_MAID_ACCOUNT_SYNC_H_

#include "maidsafe/vault/accumulator.h"
#include "maidsafe/vault/maid_account_holder/maid_account.h"

namespace maidsafe {

namespace vault {

class MaidAccountSync {
 public:
  typedef MaidName name_type;
  explicit MaidAccountSync(const MaidName& maid_name);
  DiskBasedStorage::FileIdentities AddSyncInfoUpdate(
    const NodeId& source_id,
    const MaidAccount::serialised_info_type& serialised_account_info,
    const Accumulator<MaidName>::serialised_requests& serialised_request);

  void AddDownloadedFile(DiskBasedStorage::FileIdentity file_name,
                         const NonEmptyString& file_contents);

  std::vector<DiskBasedStorage::FileIdentity> GetFileRequests(const NodeId& source_id);
  bool IsReadyForMerge();
  bool MergeSyncResults(std::unique_ptr<MaidAccount>& account, Accumulator<MaidName>& accumulator);
  MaidName name() const {return kMaidName_; }

 private:
  MaidAccountSync(MaidAccountSync&& other);

  struct SyncInfoUpdate {
    SyncInfoUpdate(
        const NodeId& sourc_id_in,
        const MaidAccount::AccountInfo& account_info_in,
        const std::vector<Accumulator<passport::PublicMaid::name_type>::HandledRequest>
            handled_requests_in,
        const DiskBasedStorage::FileIdentities shared_file_ids_in,
        const DiskBasedStorage::FileIdentities requested_file_ids_in);
    SyncInfoUpdate(const SyncInfoUpdate& other);
    SyncInfoUpdate& operator=(const SyncInfoUpdate& other);
    SyncInfoUpdate(SyncInfoUpdate&& other);
    SyncInfoUpdate& operator=(SyncInfoUpdate&& other);

    NodeId source_id;
    MaidAccount::AccountInfo account_info;
    std::vector<Accumulator<MaidName>::HandledRequest> handled_requests;
    DiskBasedStorage::FileIdentities shared_file_ids, requested_file_ids;
  };
  DiskBasedStorage::FileIdentities GetRequiredFileNames();

  mutable std::mutex mutex_;
  const name_type kMaidName_;
  std::vector<boost::filesystem::path> downloaded_files_;
  std::vector<SyncInfoUpdate> sync_updates_;
  bool is_ready_for_merge_;
};

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_MAID_ACCOUNT_HOLDER_MAID_ACCOUNT_SYNC_H_
