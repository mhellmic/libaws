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
#ifndef AWS_SQSEXCEPTION_API_H
#define AWS_SQSEXCEPTION_API_H

#include <libaws/common.h>
#include <libaws/exception.h>

namespace aws {

	class QueryErrorResponse;

	namespace sqs {
		class SQSConnection;
	}

	class SQSException : public AWSException
	{
	public:

		const std::string& getErrorMessage() { return theErrorMessage; }

		virtual const char* what() const throw();

		virtual ~SQSException() throw();

	protected:
		SQSException(const QueryErrorResponse&);

		std::string theErrorMessage;
	};

	class CreateQueueException : public SQSException
	{
		public:
			virtual ~CreateQueueException() throw();
		private:
			friend class sqs::SQSConnection;
			CreateQueueException(const QueryErrorResponse&);
	};

	class DeleteQueueException : public SQSException
	{
	public:
		virtual ~DeleteQueueException() throw();
	private:
		friend class sqs::SQSConnection;
		DeleteQueueException(const QueryErrorResponse&);
	};

	class ListQueuesException : public SQSException
	{
	public:
		virtual ~ListQueuesException() throw();
	private:
		friend class sqs::SQSConnection;
		ListQueuesException(const QueryErrorResponse&);
	};

	class SendMessageException : public SQSException
	{
	public:
		virtual ~SendMessageException() throw();
	private:
		friend class sqs::SQSConnection;
		SendMessageException(const QueryErrorResponse&);
	};

	class ReceiveMessageException : public SQSException
	{
	public:
		virtual ~ReceiveMessageException() throw();
	private:
		friend class sqs::SQSConnection;
		ReceiveMessageException(const QueryErrorResponse&);
	};

	class DeleteMessageException : public SQSException
	{
	public:
		virtual ~DeleteMessageException() throw();
	private:
		friend class sqs::SQSConnection;
		DeleteMessageException(const QueryErrorResponse&);
	};
} /* namespace aws */

#endif
