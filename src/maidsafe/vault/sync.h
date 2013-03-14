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

#ifndef MAIDSAFE_VAULT_SYNC_H_
#define MAIDSAFE_VAULT_SYNC_H_

#include <cstdint>
#include "maidsafe/common/types.h"

namespace maidsafe {

namespace vault {

class Sync {
 public:
  enum class Action : int32_t {
    kPeriodicSyncInfo,
    kSyncArchiveFiles,
    kAccountLastState,
    kTriggerAccountTransfer,
    kAccountTransfer
  };
};

struct PeriodicSyncInfo {};
struct SyncArchiveFiles {};
struct AccountLastState {};
struct TriggerAccountTransfer {};
struct AccountTransfer {};

template<Sync::Action T> struct SyncAction {};

template<> struct SyncAction<Sync::Action::kPeriodicSyncInfo> {
  typedef PeriodicSyncInfo name_type;
};

template<> struct SyncAction<Sync::Action::kSyncArchiveFiles> {
  typedef SyncArchiveFiles name_type;
};

template<> struct SyncAction<Sync::Action::kAccountLastState> {
  typedef AccountLastState name_type;
};

template<> struct SyncAction<Sync::Action::kTriggerAccountTransfer> {
  typedef TriggerAccountTransfer name_type;
};

template<> struct SyncAction<Sync::Action::kAccountTransfer> {
  typedef AccountTransfer name_type;
};

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_SYNC_H_
