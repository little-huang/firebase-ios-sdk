/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FIRESTORE_CORE_SRC_LOCAL_LEVELDB_DOCUMENT_OVERLAY_CACHE_H_
#define FIRESTORE_CORE_SRC_LOCAL_LEVELDB_DOCUMENT_OVERLAY_CACHE_H_

#include <cstdlib>
#include <functional>
#include <string>

#include "Firestore/core/src/local/document_overlay_cache.h"
#include "absl/strings/string_view.h"

namespace firebase {
namespace firestore {

namespace credentials {
class User;
}  // namespace credentials

namespace local {

class LevelDbDocumentOverlayKey;
class LevelDbPersistence;
class LocalSerializer;

class LevelDbDocumentOverlayCache final : public DocumentOverlayCache {
 public:
  LevelDbDocumentOverlayCache(const credentials::User& user,
                              LevelDbPersistence* db,
                              LocalSerializer* serializer);

  LevelDbDocumentOverlayCache(const LevelDbDocumentOverlayCache&) = delete;
  LevelDbDocumentOverlayCache& operator=(const LevelDbDocumentOverlayCache&) =
      delete;

  LevelDbDocumentOverlayCache(LevelDbDocumentOverlayCache&&) = delete;
  LevelDbDocumentOverlayCache& operator=(LevelDbDocumentOverlayCache&&) =
      delete;

  absl::optional<model::mutation::Overlay> GetOverlay(
      const model::DocumentKey& key) const override;

  void SaveOverlays(int largest_batch_id,
                    const MutationByDocumentKeyMap& overlays) override;

  void RemoveOverlaysForBatchId(int batch_id) override;

  OverlayByDocumentKeyMap GetOverlays(const model::ResourcePath& collection,
                                      int since_batch_id) const override;

  OverlayByDocumentKeyMap GetOverlays(const std::string& collection_group,
                                      int since_batch_id,
                                      std::size_t count) const override;

 private:
  model::mutation::Overlay ParseOverlay(
      const LevelDbDocumentOverlayKey& key,
      absl::string_view encoded_mutation) const;

  void SaveOverlay(int largest_batch_id,
                   const model::DocumentKey& key,
                   const model::Mutation& mutation);

  void DeleteOverlay(const model::DocumentKey& key);

  void ForEachOverlay(
      std::function<void(absl::string_view encoded_key,
                         const LevelDbDocumentOverlayKey& decoded_key,
                         absl::string_view encoded_mutation)>) const;

  // The LevelDbDocumentOverlayCache instance is owned by LevelDbPersistence.
  LevelDbPersistence* db_;

  // Owned by LevelDbPersistence.
  LocalSerializer* serializer_ = nullptr;

  /**
   * The normalized user_id (i.e. after converting null to empty) as used in our
   * LevelDB keys.
   */
  std::string user_id_;
};

}  // namespace local
}  // namespace firestore
}  // namespace firebase

#endif  // FIRESTORE_CORE_SRC_LOCAL_LEVELDB_DOCUMENT_OVERLAY_CACHE_H_
