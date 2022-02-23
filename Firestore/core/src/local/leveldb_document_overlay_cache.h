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

class LevelDbDocumentOverlayCacheTestHelper;
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

  OverlayByDocumentKeyMap GetOverlays(absl::string_view collection_group,
                                      int since_batch_id,
                                      std::size_t count) const override;

 private:
  friend class LevelDbDocumentOverlayCacheTestHelper;

  // Returns the number of index entries of the various types.
  // These methods exist for unit testing only.
  int GetLargestBatchIdIndexEntryCount() const;
  int GetCollectionIndexEntryCount() const;
  int GetCollectionGroupIndexEntryCount() const;

  int GetOverlayCount() const override;
  int CountEntriesWithKeyPrefix(const std::string& key_prefix) const;

  absl::optional<model::mutation::Overlay> GetOverlay(
      absl::string_view encoded_key,
      const LevelDbDocumentOverlayKey& decoded_key) const;

  model::mutation::Overlay ParseOverlay(
      const LevelDbDocumentOverlayKey& key,
      absl::string_view encoded_mutation) const;

  void SaveOverlay(int largest_batch_id,
                   const model::DocumentKey& key,
                   const model::Mutation& mutation);

  void DeleteOverlay(const model::DocumentKey& key);

  void DeleteOverlay(absl::string_view encoded_key,
                     const LevelDbDocumentOverlayKey& decoded_key);

  void DeleteLargestBatchIdIndexEntryFor(
      absl::string_view encoded_key,
      const LevelDbDocumentOverlayKey& decoded_key);

  void PutLargestBatchIdIndexEntryFor(
      absl::string_view encoded_key,
      const LevelDbDocumentOverlayKey& decoded_key);

  void ForEachKeyWithLargestBatchId(
      int largest_batch_id,
      std::function<void(absl::string_view encoded_key,
                         LevelDbDocumentOverlayKey&& decoded_key)>) const;

  void DeleteCollectionIndexEntryFor(
      absl::string_view encoded_key,
      const LevelDbDocumentOverlayKey& decoded_key);

  void PutCollectionIndexEntryFor(absl::string_view encoded_key,
                                  const LevelDbDocumentOverlayKey& decoded_key);

  void ForEachKeyInCollection(
      const model::ResourcePath& collection,
      int since_batch_id,
      std::function<void(absl::string_view encoded_key,
                         LevelDbDocumentOverlayKey&& decoded_key)>) const;

  void ForEachKeyInCollectionGroup(
      absl::string_view collection_group,
      int since_batch_id,
      std::function<bool(absl::string_view encoded_key,
                         LevelDbDocumentOverlayKey&& decoded_key)>) const;

  void DeleteCollectionGroupIndexEntryFor(
      absl::string_view encoded_key,
      const LevelDbDocumentOverlayKey& decoded_key);

  void PutCollectionGroupIndexEntryFor(
      absl::string_view encoded_key,
      const LevelDbDocumentOverlayKey& decoded_key);

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
