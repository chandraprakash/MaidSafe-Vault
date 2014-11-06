/*  Copyright 2013 MaidSafe.net limited

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

#ifndef MAIDSAFE_VAULT_TESTS_ACCOUNT_TRANSFER_ANALYSER_H_
#define MAIDSAFE_VAULT_TESTS_ACCOUNT_TRANSFER_ANALYSER_H_

#include <algorithm>
#include <map>
#include <mutex>
#include <utility>
#include <vector>

#include "boost/optional.hpp"

#include "maidsafe/common/node_id.h"

#include "maidsafe/vault/account_transfer_handler.h"
#include "maidsafe/vault/data_manager/data_manager.h"

namespace maidsafe {

namespace vault {

namespace test {

template <typename Persona>
class  AccountTransferInfoHandler {};

template <typename Persona>
class AccountTransferTestAnalyser : public AccountTransferInfoHandler<Persona> {
 public:
  using Value = typename Persona::Value;
  using Key = typename Persona::Key;

  using KeyValuePair = std::pair<Key, Value>;
  using Result = typename AccountTransferHandler<Persona>::Result;

  AccountTransferTestAnalyser();
  bool CheckResults(
      const std::map<Key, typename AccountTransferHandler<Persona>::Result>& results_in);

  void CreateEntries(unsigned int total);
  std::vector<KeyValuePair> GetKeyValuePairs() const;

 private:
  void Replicate();
  void ReplicateWithSameValue(typename std::vector<KeyValuePair>::iterator& resolvable_end_iter);
  void ReplicateWithDifferntValue(
      typename std::vector<KeyValuePair>::iterator& unresolvable_start_iter,
      typename std::vector<KeyValuePair>::iterator& unresolvable_end_iter);

  std::vector<KeyValuePair> kv_pairs_;
};

template <typename Persona>
AccountTransferTestAnalyser<Persona>::AccountTransferTestAnalyser()
    : kv_pairs_() {}

template <typename Persona>
bool AccountTransferTestAnalyser<Persona>::CheckResults(
    const std::map<Key, typename AccountTransferHandler<Persona>::Result>& results_in) {
  auto expected_results(AccountTransferInfoHandler<Persona>::ProduceResults(kv_pairs_));
  if (results_in.size() != expected_results.size())
    return false;
  for (const auto& expected_result : expected_results) {
    if (!expected_result.template Result::Equals(results_in.at(expected_result.key)))
      return false;
  }
  return true;
}

template <typename Persona>
void AccountTransferTestAnalyser<Persona>::CreateEntries(unsigned int total) {
  for (unsigned int index(0); index < total; ++index)
    kv_pairs_.push_back(AccountTransferInfoHandler<Persona>::CreatePair());
  Replicate();
  std::srand (unsigned(std::time(0)));
  std::random_shuffle(std::begin(kv_pairs_), std::end(kv_pairs_));
}

template <typename Persona>
std::vector<typename AccountTransferTestAnalyser<Persona>::KeyValuePair>
AccountTransferTestAnalyser<Persona>::GetKeyValuePairs() const {
  return kv_pairs_;
}

template <typename Persona>
void AccountTransferTestAnalyser<Persona>::Replicate() {
  assert(!kv_pairs_.empty());
  auto original_size(kv_pairs_.size());
  typename std::vector<KeyValuePair>::iterator resolvable_end_iter(std::begin(kv_pairs_)),
      unresolvable_iter;
  std::advance(resolvable_end_iter, original_size / 2);
  ReplicateWithSameValue(resolvable_end_iter);
  resolvable_end_iter = unresolvable_iter = std::begin(kv_pairs_);
  std::advance(resolvable_end_iter, original_size / 2 + 1);
  std::advance(unresolvable_iter, original_size * 3 / 4);
  ReplicateWithDifferntValue(resolvable_end_iter, unresolvable_iter);
}

template <typename Persona>
void AccountTransferTestAnalyser<Persona>::ReplicateWithSameValue(
    typename std::vector<KeyValuePair>::iterator& resolvable_end_iter) {
  std::vector<KeyValuePair> new_pairs;
  for (auto iter(std::begin(kv_pairs_)); iter != resolvable_end_iter; ++iter)
    for (unsigned int index(0);
         index < AccountTransferInfoHandler<Persona>::AcceptSize() - 1;
         ++index)
      new_pairs.push_back(*iter);
  std::copy(std::begin(new_pairs), std::end(new_pairs), std::back_inserter(kv_pairs_));
}

template <typename Persona>
void AccountTransferTestAnalyser<Persona>::ReplicateWithDifferntValue(
    typename std::vector<KeyValuePair>::iterator& unresolvable_start_iter,
    typename std::vector<KeyValuePair>::iterator& unresolvable_end_iter) {
  std::vector<KeyValuePair> new_pairs;
  for (; unresolvable_start_iter != unresolvable_end_iter; ++unresolvable_start_iter)
    for (unsigned int index(0);
         index < AccountTransferInfoHandler<Persona>::ResolutionSize() - 1;
         ++index) {
      new_pairs.push_back(
          std::make_pair(unresolvable_start_iter->first,
                         AccountTransferInfoHandler<Persona>::CreateValue()));
    }
  std::copy(std::begin(new_pairs), std::end(new_pairs), std::back_inserter(kv_pairs_));
}



}  // namespace test

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_TESTS_ACCOUNT_TRANSFER_ANALYSER_H_
