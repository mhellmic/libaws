#include "common.h"

#include "curlstreambuf.h"
#include "s3/s3response.h"

namespace aws { namespace s3 {

    S3Response::S3Response()
        : Response()
    {
    }

    S3Response::~S3Response()
    {
    }

    S3ResponseError&
    S3Response::getS3ResponseError()
    {
      return theS3ResponseError;
    }

    S3ResponseError::S3ResponseError()
    { }


    S3Exception::ErrorCode
    S3ResponseError::parseError ( const std::string& aString )
    {
      if ( aString.compare ( "InvalidAccessKeyId" ) ) {
        return S3Exception::InvalidAccessKeyId;
      }
      else {
        return S3Exception::NoError;
      }
    }


    CreateBucketResponse::CreateBucketResponse ( const std::string& aBucketName )
        : theBucketName ( aBucketName )
    {
      theIsSuccessful = true; // true by default because we don't get a response
    }

    CreateBucketResponse::~CreateBucketResponse()
    {
    }

    ListAllBucketsResponse::ListAllBucketsResponse()
        : S3Response()
    {
    }

    ListAllBucketsResponse::~ListAllBucketsResponse()
    {
    }

    void
    ListAllBucketsResponse::open() const
    {
      theIterator = theBuckets.begin();
    }

    bool
    ListAllBucketsResponse::next(Bucket& aBucket) const
    {
      if (theIterator != theBuckets.end()) {
        aBucket.BucketName = (*theIterator).BucketName;
        aBucket.CreationDate = (*theIterator).CreationDate;
        ++theIterator;
        return true;
      }
      return false;
    }

    void
    ListAllBucketsResponse::close() const
    {
      theIterator = theBuckets.end();
    }

    const std::string&
    ListAllBucketsResponse::getOwnerId() const
    {
      return theOwnerId;
    } 

    const std::string&
    ListAllBucketsResponse::getOwnerDisplayName() const
    {
      return theOwnerDisplayName;
    }

    ListBucketResponse::ListBucketResponse(const std::string& aBucketName, const std::string& aPrefix,
                                           const std::string& aMarker, int aMaxKeys)
        : S3Response()
    {
    }

    ListBucketResponse::~ListBucketResponse()
    {
    }

    void
    ListBucketResponse::open() const
    {
      theIterator = theKeys.begin();
    }

    bool
    ListBucketResponse::next(Key& aKey) const
    {
      if (theIterator != theKeys.end()) {
        aKey.KeyValue     = (*theIterator).KeyValue;
        aKey.LastModified = (*theIterator).LastModified;
        aKey.ETag         = (*theIterator).ETag;
        aKey.Length       = (*theIterator).Length;
        ++theIterator;
        return true;
      }
      return false;
    }

    void
    ListBucketResponse::close() const
    {
      theIterator = theKeys.end();
    }

    DeleteBucketResponse::DeleteBucketResponse ( const std::string& aBucketName )
        : S3Response(),
        theBucketName ( aBucketName )
    {
      theIsSuccessful = true;
    }

    DeleteBucketResponse::~DeleteBucketResponse()
    {
    }

    PutResponse::PutResponse ( const std::string& aBucketName )
        : theBucketName ( aBucketName )
    {
    }

    PutResponse::~PutResponse()
    {
    }


    GetResponse::GetResponse ( const std::string& aBucketName, const std::string& aKey )
        : theBucketName ( aBucketName ),
          theKey ( aKey ),
          theContentSize ( 0 ),
          theStreamBuffer( 0 ),
          theInputStream( 0 ),
          theIsModified(true)
    {
    }

    GetResponse::~GetResponse()
    {
      delete theInputStream;
      delete theStreamBuffer;
    }


    HeadResponse::HeadResponse ( const std::string& aBucketName )
        : theBucketName ( aBucketName )
    {
    }

    HeadResponse::~HeadResponse()
    {
    }


    DeleteResponse::DeleteResponse ( const std::string& aBucketName,
                                     const std::string& aKey )
        : theBucketName ( aBucketName ),
        theKey ( aKey )
    {
    }

    DeleteResponse::~DeleteResponse()
    {
    }


} } // end namespaces
