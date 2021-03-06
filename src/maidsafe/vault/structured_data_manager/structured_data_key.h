/* Copyright 2013 MaidSafe.net limited

This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or later,
and The General Public License (GPL), version 3. By contributing code to this project You agree to
the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the root directory
of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available at:

http://www.novinet.com/license

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing permissions and limitations under the
License.
*/

#ifndef MAIDSAFE_VAULT_STRUCTURED_DATA_MANAGER_STRUCTURED_DATA_KEY_H_
#define MAIDSAFE_VAULT_STRUCTURED_DATA_MANAGER_STRUCTURED_DATA_KEY_H_

#include <string>

#include "maidsafe/data_types/data_name_variant.h"
#include "maidsafe/nfs/persona_id.h"
#include "maidsafe/nfs/types.h"


namespace maidsafe {

namespace vault {

class Db;
template<typename PersonaType>
class ManagerDb;

class StructuredDataKey {
 public:
  StructuredDataKey();
  StructuredDataKey(const DataNameVariant& data_name, const Identity& originator);
  StructuredDataKey(const StructuredDataKey& other);
  StructuredDataKey(StructuredDataKey&& other);
  StructuredDataKey& operator=(StructuredDataKey other);

  DataNameVariant data_name() const { return data_name_; }
  Identity originator() const { return originator_; }

  friend void swap(StructuredDataKey& lhs, StructuredDataKey& rhs) MAIDSAFE_NOEXCEPT;
  friend bool operator==(const StructuredDataKey& lhs, const StructuredDataKey& rhs);
  friend bool operator<(const StructuredDataKey& lhs, const StructuredDataKey& rhs);
  friend class Db;
  template<typename PersonaType>
  friend class ManagerDb;

 private:
  explicit StructuredDataKey(const std::string& serialised_key);
  std::string Serialise() const;
  DataNameVariant data_name_;
  Identity originator_;
  static const int kPaddedWidth_;
};

bool operator!=(const StructuredDataKey& lhs, const StructuredDataKey& rhs);

bool operator>(const StructuredDataKey& lhs, const StructuredDataKey& rhs);

bool operator<=(const StructuredDataKey& lhs, const StructuredDataKey& rhs);

bool operator>=(const StructuredDataKey& lhs, const StructuredDataKey& rhs);

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_STRUCTURED_DATA_MANAGER_STRUCTURED_DATA_KEY_H_
