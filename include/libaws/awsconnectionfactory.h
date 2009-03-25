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
#ifndef LIBAWS_AWSCONNECTIONFACTORY_API_H
#define LIBAWS_AWSCONNECTIONFACTORY_API_H

#include <string>
#include <libaws/common.h>

namespace aws {

  /*! \brief Singleton factory for creating instances
   *         of the aws::s3::S3Connection and aws::sqs::SQSConnection classes.
   *
   * The AWSConnectionFactory is the single point of entry into the AWS C++ library.
   * It's main purposes are to:
   * <ol>
   *  <li>initialize the AWS C++ library and retrieve the singleton instance of
   *      the aws::AWSConnectionFactory (see getInstance()).</li>
   *  <li>create connections to the <a href="http://www.amazon.com/s3">Amazon AWS S3 Service</a>.</li>
   *  <li>shutdown the AWS C++ library (see shutdown()).</li>
   * </ol>
   */
  class AWSConnectionFactory
  {
  public:

    /*! \brief Retrieve the singleton instance.
     *
     *  This static function provides the singleton instance of the aws::AWSConnectionFactory.
     *  On the first call, it initializes the data structures that are needed during the use
     *  of the AWS C++ library. For example, it initializes the
     *  <a href="http://curl.haxx.se/">libcurl</a> and
     *  <a href="http://xmlsoft.org/">libxml2</a> libraries.
     *  Because initialization of libcurl may fail the function can fail the AWS C++ library
     *  may be unusable. In this case, an error is reported if trying to create an S3 or SQS
     *  connection.
     *
     * \throws AWSConnectionException if the initialization of libcurl failed.
     *
     * @return The singleton instance of the aws::AWSConnectionFactory;
     */
    static AWSConnectionFactory*
    getInstance();

    /*! \brief Retrieve a smart pointer to a aws::s3::S3Connection instance.
     *
     * The createS3Connection function creates an instance of the aws::s3::S3Connection class.
     * Such an instance provides operations on the AWS S3 service (e.g. creating a bucket or
     * storing an object on S3). See the documentation of aws::s3::S3Connection for a complete
     * reference of the operations provided by this class
     *
     * Note that the use of such an object is restricted to one thread only.
     * However, multiple instance can be created and every instance can be used in a
     * different thread.
     *
     * \throws aws::AWSInitializationException if the initialization of libcurl fails
     *         during the first call of getInstance (or any directly following getInstance call
     *         after shutdown has been called). In this case, libaws cannot be
     *         used. See <a href="http://curl.haxx.se">libcurl</a> for more information
     *         about the error that is reported.
     * \throws aws::AWSAccessKeyIdMissingException if the AWS Access Key Id provided as parameter
     *         is empty.
     * \throws aws::AWSSecretAccessKeyMissingException if the AWS Secret Access Key provided
     *         as parameter is empty.
     *
     * @return A smart pointer to a aws::s3::S3Connection instance.
     */
    virtual S3ConnectionPtr
    createS3Connection(const std::string& aAccessKeyId,  const std::string& aSecretAccessKey,
                       const std::string& aCustomHost = "") const = 0;

    /*! \brief Retrieve a smart pointer to a aws::sqs::SQSConnection instance.
     *
     * The createSQSConnection function creates an instance of the aws::sqs::SQSConnection class.
     * Such an instance provides operations on the AWS SQS service (e.g. creating a queue or
     * receiving a mesage from an SQS queue). See the documentation of aws::sqs::SQSConnection
     * for a complete reference of the operations provided by this class.
     *
     * Note that the use of such an object is restricted to one thread only.
     * However, multiple instance can be created and every instance can be used in a
     * different thread.
     *
     * \throws aws::AWSInitializationException if the initialization of libcurl fails
     *         during the first call of getInstance (or any directly following getInstance call
     *         after shutdown has been called). In this case, libaws cannot be
     *         used. See <a href="http://curl.haxx.se">libcurl</a> for more information
     *         about the error that is reported.
     * \throws aws::AWSAccessKeyIdMissingException if the AWS Access Key Id provided as parameter
     *         is empty.
     * \throws aws::AWSSecretAccessKeyMissingException if the AWS Secret Access Key provided
     *         as parameter is empty.
     *
     * @return A smart pointer to a aws::sqs::SQSConnection instance.
     */
    virtual SQSConnectionPtr
    createSQSConnection(const std::string& aAccessKeyId, const std::string& aSecretAccessKey,
                        const std::string& aCustomHost = "") const = 0;


    /*! \brief Retrieve a smart pointer to a aws::sqs::SQSConnection instance.
     *
     * The createSDBConnection function creates an instance of the aws::sqs::SDBConnection class.
     * Such an instance provides operations on the AWS SDB service (e.g. creating a domain or
     * querying). See the documentation of aws::sdb::SDBConnection
     * for a complete reference of the operations provided by this class.
     *
     * Note that the use of such an object is restricted to one thread only.
     * However, multiple instance can be created and every instance can be used in a
     * different thread.
     *
     * \throws aws::AWSInitializationException if the initialization of libcurl fails
     *         during the first call of getInstance (or any directly following getInstance call
     *         after shutdown has been called). In this case, libaws cannot be
     *         used. See <a href="http://curl.haxx.se">libcurl</a> for more information
     *         about the error that is reported.
     * \throws aws::AWSAccessKeyIdMissingException if the AWS Access Key Id provided as parameter
     *         is empty.
     * \throws aws::AWSSecretAccessKeyMissingException if the AWS Secret Access Key provided
     *         as parameter is empty.
     *
     * @return A smart pointer to a aws::sdb::SDBConnection instance.
     */
    virtual SDBConnectionPtr
    createSDBConnection(const std::string& aAccessKeyId, const std::string& aSecretAccessKey,
                        const std::string& aCustomHost = "") const = 0;

    /*! \brief Release all resources that have been allocated by libaws or any library it uses.
     *
     * This function releases all resources that have been allocated by libaws
     * or any library libaws uses (i.e. libcurl, libxml2).
     *
     * Note that libcurl and libxml2 have to be reinitialized if they are used in the
     * application that called shutdown.
     *
     * If shutdown is not called explicitly by an application it will be called implicitly
     * at the end of the process.
     */
    virtual void
    shutdown() = 0;

    /*! \brief Destructor that is called at the end of the process (deinitialization of static data).
     *
     *  The destructor is called at the end of process. It implicitly calls shutdown
     *  it that has not been done before
     */
    virtual ~AWSConnectionFactory();

    /*! \brief AWS C++ library version.
     *         see aws::AWSVersion for more information.
     */
    virtual std::string getVersion() = 0;

    /*! \brief Retrieve a smart pointer to a aws::sqs::SQSConnection instance.
     *
     * The createSQSConnection function creates an instance of the aws::sqs::SQSConnection class.
     * Such an instance provides operations on the AWS SQS service (e.g. creating a queue or
     * receiving a mesage from an SQS queue). See the documentation of aws::sqs::SQSConnection
     * for a complete reference of the operations provided by this class.
     *
     * Note that the use of such an object is restricted to one thread only.
     * However, multiple instance can be created and every instance can be used in a
     * different thread.
     *
     * \throws aws::AWSInitializationException if the initialization of libcurl fails
     *         during the first call of getInstance (or any directly following getInstance call
     *         after shutdown has been called). In this case, libaws cannot be
     *         used. See <a href="http://curl.haxx.se">libcurl</a> for more information
     *         about the error that is reported.
     * \throws aws::AWSAccessKeyIdMissingException if the AWS Access Key Id provided as parameter
     *         is empty.
     * \throws aws::AWSSecretAccessKeyMissingException if the AWS Secret Access Key provided
     *         as parameter is empty.
     *
     * @return A smart pointer to a aws::sqs::SQSConnection instance.
     */
    virtual SQSConnectionPtr
    createSQSConnection(const std::string& aAccessKeyId, const std::string& aSecretAccessKey,
    		const std::string& aCustomHost, int aPort, bool aIsSecure) const = 0;
  };

} /* namespace aws */
#endif
