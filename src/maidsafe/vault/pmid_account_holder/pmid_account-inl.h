/***************************************************************************************************
 *  Copyright 2013 MaidSafe.net limited                                                            *
 *                                                                                                 *
 *  The following source code is property of MaidSafe.net limited and is not meant for external    *
 *  use.  The use of this code is governed by the licence file licence.txt found in the root of    *
 *  this directory and also on www.maidsafe.net.                                                   *
 *                                                                                                 *
 *  You are not free to copy, amend or otherwise use this source code without the explicit         *
 *  written permission of the board of directors of MaidSafe.net.                                  *
 **************************************************************************************************/

#ifndef MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_INL_H_
#define MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_INL_H_

#include "maidsafe/vault/utils.h"


namespace maidsafe {

namespace vault {

template<typename Data>
void PmidAccount::PutData(const typename Data::name_type& name, int32_t size) {
  auto store_future(data_held_.Store<Data>(name, size));
  store_future.get();
  historic_stored_space_ += size;
}

template<typename Data>
void PmidAccount::DeleteData(const typename Data::name_type& name) {
  auto delete_future(data_held_.Delete<Data>(name));
  auto size(delete_future.get());
  historic_stored_space_ -= size;
}

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_INL_H_
