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
#ifndef AWS_SDB_SDBHANDLER_H
#define AWS_SDB_SDBHANDLER_H

#include "awsquerycallback.h"

namespace aws {
	namespace sdb {

		class SDBResponse;
		class CreateDomainResponse;
		class DeleteDomainResponse;
		class DomainMetadataResponse;
		class ListDomainsResponse;
		class PutAttributesResponse;
    class BatchPutAttributesResponse;
		class DeleteAttributesResponse;
		class GetAttributesResponse;
		class SDBQueryResponse;
    class SDBQueryWithAttributesResponse;

		template<class T>
		class SDBHandler: public SimpleQueryCallBack {
			friend class SDBConnection;

		protected:
			T* theResponse;

			virtual void responseStartElement(const xmlChar * localname,
					int nb_attributes, const xmlChar ** attributes) = 0;
			virtual void responseCharacters(const xmlChar * value, int len) = 0;
			virtual void responseEndElement(const xmlChar * localname) = 0;

		public:
			enum States {
				ERROR_Code = 1 << 0,
				ERROR_Message = 1 << 1,
				RequestId = 1 << 2,
				HostId = 1 << 3,
				BoxUsage = 1 << 4,
				DomainName = 1 << 5,
				NextToken = 1 << 6,
				Name = 1 << 7,
				Value = 1 << 8,
				Item = 1 << 9,
				Attribute = 1 << 10, 
				AttributeName = 1 << 11,
				ItemName = 1 << 12,
				AttributeValue = 1 << 13,
        ItemCount = 1 << 14,
        ItemNamesSizeBytes = 1 << 15,
        AttributeNameCount = 1 << 16,
        AttributeNamesSizeBytes = 1 << 17,
        AttributeValueCount = 1 << 18,
        AttributeValuesSizeBytes = 1 << 19,
        Timestamp = 1 << 20
			};

			SDBHandler() : theResponse(0) {
			};

			void startElement(const xmlChar * localname, int nb_attributes,
					const xmlChar ** attributes);
			void characters(const xmlChar * value, int len);
			void endElement(const xmlChar * localname);
		};

		class CreateDomainHandler: public SDBHandler<CreateDomainResponse> {
		protected:

		public:
			virtual void responseStartElement(const xmlChar * localname,
					int nb_attributes, const xmlChar ** attributes);
			virtual void responseCharacters(const xmlChar * value, int len);
			virtual void responseEndElement(const xmlChar * localname);

		};

		class DeleteDomainHandler: public SDBHandler<DeleteDomainResponse> {
		protected:

		public:
			virtual void responseStartElement(const xmlChar * localname,
					int nb_attributes, const xmlChar ** attributes);
			virtual void responseCharacters(const xmlChar * value, int len);
			virtual void responseEndElement(const xmlChar * localname);

		};

		class DomainMetadataHandler: public SDBHandler<DomainMetadataResponse> {
		protected:

		public:
			virtual void responseStartElement(const xmlChar * localname,
					int nb_attributes, const xmlChar ** attributes);
			virtual void responseCharacters(const xmlChar * value, int len);
			virtual void responseEndElement(const xmlChar * localname);

		};

		class ListDomainsHandler: public SDBHandler<ListDomainsResponse> {
		protected:

		public:
			virtual void responseStartElement(const xmlChar * localname,
					int nb_attributes, const xmlChar ** attributes);
			virtual void responseCharacters(const xmlChar * value, int len);
			virtual void responseEndElement(const xmlChar * localname);

		};

		class PutAttributesHandler: public SDBHandler<PutAttributesResponse> {
		protected:

		public:
			virtual void responseStartElement(const xmlChar * localname,
					int nb_attributes, const xmlChar ** attributes);
			virtual void responseCharacters(const xmlChar * value, int len);
			virtual void responseEndElement(const xmlChar * localname);

		};

		class BatchPutAttributesHandler: public SDBHandler<BatchPutAttributesResponse> {
		protected:

		public:
			virtual void responseStartElement(const xmlChar * localname,
					int nb_attributes, const xmlChar ** attributes);
			virtual void responseCharacters(const xmlChar * value, int len);
			virtual void responseEndElement(const xmlChar * localname);

		};

		class DeleteAttributesHandler: public SDBHandler<DeleteAttributesResponse> {
		protected:

		public:
			virtual void responseStartElement(const xmlChar * localname,
					int nb_attributes, const xmlChar ** attributes);
			virtual void responseCharacters(const xmlChar * value, int len);
			virtual void responseEndElement(const xmlChar * localname);

		};

		class GetAttributesHandler: public SDBHandler<GetAttributesResponse> {
		protected:

		public:
			virtual void responseStartElement(const xmlChar * localname,
					int nb_attributes, const xmlChar ** attributes);
			virtual void responseCharacters(const xmlChar * value, int len);
			virtual void responseEndElement(const xmlChar * localname);

		};

		class QueryHandler: public SDBHandler<SDBQueryResponse> {
		protected:

		public:
			virtual void responseStartElement(const xmlChar * localname,
					int nb_attributes, const xmlChar ** attributes);
			virtual void responseCharacters(const xmlChar * value, int len);
			virtual void responseEndElement(const xmlChar * localname);

		};

		class QueryWithAttributesHandler: public SDBHandler<SDBQueryWithAttributesResponse> {
		protected:

		public:
			virtual void
      responseStartElement(const xmlChar * localname, int nb_attributes, const xmlChar ** attributes);

			virtual void
      responseCharacters(const xmlChar * value, int len);

			virtual void
      responseEndElement(const xmlChar * localname);

		};

	} /* namespace sdb  */
} /* namespace aws */

#endif
