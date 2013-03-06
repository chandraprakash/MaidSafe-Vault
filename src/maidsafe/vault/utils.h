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

#ifndef MAIDSAFE_VAULT_UTILS_H_
#define MAIDSAFE_VAULT_UTILS_H_

#include <memory>
#include <mutex>
#include <vector>

#include "boost/filesystem/path.hpp"

#include "maidsafe/common/error.h"
#include "maidsafe/data_types/data_name_variant.h"
#include "maidsafe/routing/routing_api.h"
#include "maidsafe/nfs/data_message.h"

#include "maidsafe/vault/types.h"


namespace maidsafe {

namespace vault {

template<typename Message>
inline bool FromMaidAccountHolder(const Message& message);

template<typename Message>
inline bool FromMetadataManager(const Message& message);

template<typename Message>
inline bool FromPmidAccountHolder(const Message& message);

template<typename Message>
inline bool FromDataHolder(const Message& message);

template<typename Message>
inline bool FromClientMaid(const Message& message);

template<typename Message>
inline bool FromClientMpid(const Message& message);

template<typename Message>
inline bool FromOwnerDirectoryManager(const Message& message);

template<typename Message>
inline bool FromGroupDirectoryManager(const Message& message);

template<typename Message>
inline bool FromWorldDirectoryManager(const Message& message);

template<typename Message>
inline bool FromDataGetter(const Message& message);


namespace detail {

void InitialiseDirectory(const boost::filesystem::path& directory);

bool ShouldRetry(routing::Routing& routing, const nfs::DataMessage& data_message);

MaidName GetSourceMaidName(const nfs::DataMessage& data_message);

template<typename Data>
bool IsDataElement(const typename Data::name_type& name,
                   const DataNameVariant& data_name_variant);

void SendReply(const nfs::DataMessage& original_message,
               const maidsafe_error& return_code,
               const routing::ReplyFunctor& reply_functor);

// Ensure the mutex protecting accounts is locked throughout this call
template<typename Account>
typename std::vector<std::unique_ptr<Account>>::iterator FindAccount(
    std::vector<std::unique_ptr<Account>>& accounts,
    const typename Account::name_type& account_name);

// Ensure the mutex protecting accounts is locked throughout this call
template<typename Account>
typename std::vector<std::unique_ptr<Account>>::const_iterator FindAccount(
    const std::vector<std::unique_ptr<Account>>& accounts,
    const typename Account::name_type& account_name);

template<typename Account>
bool AddAccount(std::mutex& mutex,
                std::vector<std::unique_ptr<Account>>& accounts,
                std::unique_ptr<Account>&& account);

template<typename Account>
bool DeleteAccount(std::mutex& mutex,
                   std::vector<std::unique_ptr<Account>>& accounts,
                   const typename Account::name_type& account_name);

template<typename Account>
typename Account::serialised_type GetSerialisedAccount(
    std::mutex& mutex,
    const std::vector<std::unique_ptr<Account>>& accounts,
    const typename Account::name_type& account_name);

template<typename Account>
typename Account::serialised_info_type GetSerialisedAccountSyncInfo(
    std::mutex& mutex,
    const std::vector<std::unique_ptr<Account>>& accounts,
    const typename Account::name_type& account_name);

// Returns true if the required successful request count has been reached
template<typename Accumulator>
bool AddResult(const nfs::DataMessage& data_message,
               const routing::ReplyFunctor& reply_functor,
               const maidsafe_error& return_code,
               Accumulator& accumulator,
               std::mutex& accumulator_mutex,
               int requests_required);

}  // namespace detail

}  // namespace vault

}  // namespace maidsafe

#include "maidsafe/vault/utils-inl.h"

#endif  // MAIDSAFE_VAULT_UTILS_H_
