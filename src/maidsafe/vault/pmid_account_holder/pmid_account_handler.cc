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

#include "maidsafe/vault/pmid_account_holder/pmid_account_handler.h"

#include <algorithm>

#include "boost/filesystem/operations.hpp"
#include "maidsafe/vault/pmid_account_holder/pmid_record.h"

#include "maidsafe/vault/utils.h"


namespace fs = boost::filesystem;

namespace maidsafe {

namespace vault {

PmidAccountHandler::PmidAccountHandler(const fs::path& vault_root_dir)
    : kPmidAccountsRoot_(vault_root_dir / "pmids"),
      mutex_(),
      pmid_accounts_() {
  detail::InitialiseDirectory(kPmidAccountsRoot_);
}

PmidAccountHandler::AccountHealth PmidAccountHandler::GetAccountHealth(const PmidName& account_name) {
  PmidRecord pmid_record;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto itr(detail::FindAccount(pmid_accounts_, account_name));
    if (itr == pmid_accounts_.end())
      ThrowError(VaultErrors::no_such_account);
    pmid_record = (*itr)->pmid_record();
  }
  AccountHealth account_health;
  int64_t delta(std::max(pmid_record.historic_stored_space - pmid_record.historic_lost_space,
                         int64_t(0)));
  double status(delta / pmid_record.historic_stored_space);
  account_health.health = 100 * status;
  account_health.offered_space = (status * pmid_record.claimed_available_size) + delta;
  return account_health;
}

bool PmidAccountHandler::AddAccount(std::unique_ptr<PmidAccount> pmid_account) {
  return detail::AddAccount(mutex_, pmid_accounts_, std::move(pmid_account));
}

bool PmidAccountHandler::DeleteAccount(const PmidName& account_name) {
  return detail::DeleteAccount(mutex_, pmid_accounts_, account_name);
}

void PmidAccountHandler::SetDataHolderUp(const PmidName& account_name) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto itr(detail::FindAccount(pmid_accounts_, account_name));
  if (itr == pmid_accounts_.end())
    ThrowError(VaultErrors::no_such_account);
  (*itr)->SetDataHolderUp();
}

void PmidAccountHandler::SetDataHolderDown(const PmidName& account_name) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto itr(detail::FindAccount(pmid_accounts_, account_name));
  if (itr == pmid_accounts_.end())
    ThrowError(VaultErrors::no_such_account);
  (*itr)->SetDataHolderDown();
}

bool PmidAccountHandler::DataHolderOnline(const PmidName& account_name) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto itr(detail::FindAccount(pmid_accounts_, account_name));
  if (itr == pmid_accounts_.end())
    ThrowError(VaultErrors::no_such_account);
  return (*itr)->DataHolderOnline();
}

std::vector<PmidName> PmidAccountHandler::GetAccountNames() const {
  std::vector<PmidName> account_names;
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto& pmid_account : pmid_accounts_)
    account_names.push_back(pmid_account->name());
  return account_names;
}

std::vector<PmidName> PmidAccountHandler::GetArchivedAccountNames() const {
  std::vector<PmidName> archived_names;
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto& archived_account : archived_accounts_)
    archived_names.push_back(archived_account);
  return archived_names;
}

PmidAccount::serialised_type PmidAccountHandler::GetSerialisedAccount(
    const PmidName& account_name) const {
  return detail::GetSerialisedAccount(mutex_, pmid_accounts_, account_name);
}

std::vector<fs::path> PmidAccountHandler::GetArchiveFileNames(
    const PmidName& account_name) const {
  std::lock_guard<std::mutex> lock(mutex_);
  auto itr(detail::FindAccount(pmid_accounts_, account_name));
  if (itr == pmid_accounts_.end())
    ThrowError(VaultErrors::no_such_account);
  return (*itr)->GetArchiveFileNames();
}

NonEmptyString PmidAccountHandler::GetArchiveFile(const PmidName& account_name,
                                                  const fs::path& path) const {
  std::lock_guard<std::mutex> lock(mutex_);
  auto itr(detail::FindAccount(pmid_accounts_, account_name));
  if (itr == pmid_accounts_.end())
    ThrowError(VaultErrors::no_such_account);
  return (*itr)->GetArchiveFile(path);
}

void PmidAccountHandler::PutArchiveFile(const PmidName& account_name,
                                        const fs::path& path,
                                        const NonEmptyString& content) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto itr(detail::FindAccount(pmid_accounts_, account_name));
  if (itr == pmid_accounts_.end())
    ThrowError(VaultErrors::no_such_account);
  (*itr)->PutArchiveFile(path, content);
}

void PmidAccountHandler::MoveToArchive(const PmidName& account_name) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto itr(detail::FindAccount(pmid_accounts_, account_name));
  if (itr != pmid_accounts_.end()) {
    serialise to disk
    pmid_accounts_.erase(itr);
  }

  rename account dir to "index.account_name"
}

void PmidAccountHandler::MoveFromArchive(const PmidName& account_name) {
  std::lock_guard<std::mutex> lock(mutex_);
  find on disk
  remove index portion from account dir name
  parse
  add to pmid_accounts_
//  auto archive_itr(std::find(archived_accounts_.begin(), archived_accounts_.end(), account_name));
//  if (archive_itr == archived_accounts_.end())
//    ThrowError(VaultErrors::no_such_account);

//  pmid_accounts_.push_back(std::unique_ptr<PmidAccount>(new PmidAccount(account_name,
//                                                                        kPmidAccountsRoot_)));
}

}  // namespace vault

}  // namespace maidsafe
