/*
 * Copyright 2008 28msec, Inc.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef AWS_S3_S3CONNECTION_API_H
#define AWS_S3_S3CONNECTION_API_H

#include <istream>
#include <map>
#include <libaws/common.h>

namespace aws {

  class S3Connection : public SmartObject
  {
    public:
      virtual ~S3Connection() {}

      /*! \brief Creates a bucket on S3
       *
       * This function creates a bucket on S3. The name of the bucket to create
       * is passed as a parameter to this function.
       *
       * @param aBucketName The name of the bucket to create.
       * @returns aws::s3::CreateBucketResponse containing information about the bucket that
       *          was created.
       *
       * \throws aws::s3::CreateBucketException if the bucket could not be created successfully.
       * \throws aws::AWSConnectionException if a connection error occured.
       */
      virtual CreateBucketResponsePtr
      createBucket(const std::string& aBucketName) = 0;

      /*! \brief Gets all buckets that belong to the caller's account
       *
       * This function receives all buckets that belong to the caller's AWS account.
       *
       * @returns aws::s3::ListAllBucketsResponsePtr containing a list of all buckets
       *          belonging to the caller's account.
       *
       * \throws aws::s3::ListAllBucketsException if the list of all buckets could not be received.
       * \throws aws::AWSConnectionException if a connection error occured.
       */
      virtual ListAllBucketsResponsePtr
      listAllBuckets() = 0;

      /*! \brief Deletes a bucket on S3
       *
       * This function deletes a bucket on S3. The name of the bucket to delete
       * is passed as a parameter to this function.
       *
       * @param aBucketName The name of the bucket to delete.
       * @returns aws::s3::DeleteBucketResponse containing information about the bucket that
       *          was delete.
       *
       * \throws DeleteBucketException 
       * \throws aws::AWSConnectionException if a connection error occured.
       */
      virtual DeleteBucketResponsePtr
      deleteBucket(const std::string& aBucketName) = 0;

      /*! \brief List information about the objects in a given bucket.
       *
       * For more information see http://docs.amazonwebservices.com/AmazonS3/2006-03-01/ and
       * http://docs.amazonwebservices.com/AmazonS3/2006-03-01/ListingKeysRequest.html.
       *
       * @param aBucketName The name of the bucket.
       * @param aPrefix Limits the response to keys which begin with the indicated prefix.
       * @param aMarker Indicates where in the bucket to begin listing. 
       *                The list will only include keys that occur lexicographically after marker.
       * @param aMaxKeys The maximum number of keys you'd like to see in the response body.
       * @param aDelimiter Causes keys that contain the same string between the prefix and the first occurrence 
       *                   of the delimiter to be rolled up the CommonPrefixes set in the Response.
       *
       * @returns aws::s3::ListBucketRespose
       *
       * \throws ListBucketException
       * \throws aws::AWSConnectionException if a connection error occured.
       */
      virtual ListBucketResponsePtr
      listBucket(const std::string& aBucketName, 
                 const std::string& aPrefix ="", 
                 const std::string& aMarker ="", 
                 const std::string& aDelimiter = "",
                 int aMaxKeys = -1) = 0;

      /*! \brief Put an object on S3.
       *
       * Stores an object given in an input stream on S3. The object is stored in the given bucket using the given key.
       *
       * @param aBucketName The name of the bucket the object should be stored in.
       * @param aKey The name of the key the object should be stored with.
       * @param aData The object to store as an input stream.
       * @param aContentType The content type of the object to store.
       * @param aSize An optional parameter specificying the size of the object.
       *              If -1 is passed, seek is used on the input stream to determine the size
       *              of the object to send.
       *
       * \throws aws::s3::PutException if the object couldn't be stored.
       * \throws aws::AWSConnectionException if a connection error occured.
       */
      virtual PutResponsePtr
      put(const std::string& aBucketName,
          const std::string& aKey,
          std::istream& aData,
          const std::string& aContentType,
          const std::map<std::string, std::string>* aMetaDataMap = 0,
          long aSize = -1) = 0;


      /*! \brief Put an object on S3.
       *
       * Stores an object given as a char pointer on S3. The object is stored in the given bucket using the given key.
       * 
       * @param aBucketName The name of the bucket the object should be stored in.
       * @param aKey The name of the key the object should be stored with.
       * @param aData The object to store as a char pointer.
       * @param aContentType The content type of the object to store.
       * @param aSize Parameter specificying the size of the object.
       *
       * \throws aws::s3::PutException if the object couldn't be stored
       * \throws aws::AWSConnectionException if a connection error occured.
       */
       virtual PutResponsePtr
       put(const std::string& aBucketName,
           const std::string& aKey,
           const char* aData,
           const std::string& aContentType,
           long aSize,
           const std::map<std::string, std::string>* aMetaDataMap = 0) = 0;

      /*! \brief Create a get query string for encoding in a web page.
       *
       * This function creates a query string (URL) that will allow a file
       * with the given key to be retrieved from the given bucket.  The URL is
       * only valid until the expiration time provided.
       *
       * @param aBucketName The name of the bucket in which the object is
       *   stored.
       * @param aKey The key for which the object should be retrieved.
       * @param aExpiration The time at which the URL expires (seconds since
       *   the Epoch)
       *
       * \throws aws::AWSConnectionException if a connection error occured.
       */
      virtual std::string
      getQueryString(const std::string& aBucketName,
                     const std::string& aKey,
                     time_t aExpiration) = 0;

      /*! \brief Receive an object from S3.
       *
       * This function receives and object from S3. The object is retrieved from the
       * given bucket with the given key.
       *
       * @param aBucketName The name of the bucket in which the object is stored.
       * @param aKey The key for which the object should be retrieved.
       * @param aMetaDataMap A map containing additional headers that are sent 
       *                     with the get requests (e.g. Range:)
       *
       *
       * \throws aws::s3::GetException if the object coldn't be received.
       * \throws aws::AWSConnectionException if a connection error occured.
       */
      virtual GetResponsePtr
      get(const std::string& aBucketName,
          const std::string& aKey,
          const std::map<std::string, std::string>* aMetaDataMap = 0) = 0;

      /*! \brief Receive an object from S3 if the given ETag has changed.
       *
       * This function receives and object from S3. The object is only retrieved from the
       * given bucket with the given key if the ETag on S3 is different then the
       * given ETag.
       *
       * @param aBucketName The name of the bucket in which the object is stored.
       * @param aKey The key for which the object should be retrieved.
       * @param aOldETag The ETag of an object with the given key. The object
       *        is only retrieved if the ETag for the according object is different on
       *        S3.
       *
       * \throws aws::s3::GetException if the object coldn't be received.
       * \throws aws::AWSConnectionException if a connection error occured.
       */
      virtual GetResponsePtr
      get(const std::string& aBucketName,
          const std::string& aKey,
          const std::string& aOldEtag) = 0;

      /*! \brief Delete an object from S3. 
       *
       * This function delete an object in the given bucket with the given key from S3.
       *
       * @param aBucketName The name of the bucket in which the object is stored.
       * @param aKey The key for which the object to be deleted.
       *
       * \throws aws::s3::DeleteException if the object coldn't be deleted.
       * \throws aws::AWSConnectionException if a connection error occured.
       */
      virtual DeleteResponsePtr
      del(const std::string& aBucketName,
          const std::string& aKey) = 0;

      /*! \brief Delete all objects in a bucket that share the same prefix.
       *
       * This function deletes all objects in the given bucket whose keys have the same prefix.
       *
       * @param aBucketName The name of the bucket whose keys should be deleted.
       * @param aPrefix The prefix of the keys that should be deleted.
       *
       * \throws aws::s3::DeleteException if the object coldn't be deleted.
       * \throws aws::AWSConnectionException if a connection error occured.
       */
      virtual DeleteAllResponsePtr
      deleteAll(const std::string& aBucketName,
              const std::string& aPrefix = "") = 0;

      virtual HeadResponsePtr
      head(const std::string& aBucketName,
          const std::string& aKey) = 0;

  }; /* class S3Connection */

} /* namespace aws */
#endif
