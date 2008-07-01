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
#ifndef AWS_S3_HANDLER_H
#define AWS_S3_HANDLER_H

#include "common.h"

#include <fstream>
#include <libxml/parser.h>


namespace aws { namespace s3   {

class S3Handler
{
public:
    S3Handler();
    
    void setState(uint64_t s)   { theCurrentState |= s; }
    bool isSet(uint64_t s)      { return theCurrentState & s; }
    void unsetState(uint64_t s) { theCurrentState ^= s; }
    
protected:
    uint64_t theCurrentState;
};

class CreateBucketHandler  : public S3Handler
{
public:
    CreateBucketHandler();

protected:
    enum States {
        Code        = 1,
        Message     = 2,
        RequestId   = 4,
        HostId      = 8
    };

public:
    static void startElementNs( void * ctx, 
                                const xmlChar * localname, 
                                const xmlChar * prefix, 
                                const xmlChar * URI, 
                                int nb_namespaces, 
                                const xmlChar ** namespaces, 
                                int nb_attributes, 
                                int nb_defaulted, 
                                const xmlChar ** attributes );
    
    static void	charactersSAXFunc(void * ctx, 
    					          const xmlChar * value, 
                                  int len);
    
    static void	endElementNs(void * ctx, 
    					     const xmlChar * localname, 
    					     const xmlChar * prefix, 
                   const xmlChar * URI);
};

class ListAllBucketsHandler  : public S3Handler
{
public:
    ListAllBucketsHandler();

protected:
    enum States {
        Owner          = 1,
        Id             = 2,
        DisplayName    = 4,
        Buckets        = 8,
        Bucket         = 16,
        Name           = 32,
        CreationDate   = 64,
        Error          = 128,
        Code           = 256,
        Message        = 512,
    };
    
    
public:
    static void startElementNs( void * ctx, 
                                const xmlChar * localname, 
                                const xmlChar * prefix, 
                                const xmlChar * URI, 
                                int nb_namespaces, 
                                const xmlChar ** namespaces, 
                                int nb_attributes, 
                                int nb_defaulted, 
                                const xmlChar ** attributes );
    
    static void	charactersSAXFunc(void * ctx, 
    					          const xmlChar * value, 
                                  int len);
    
    static void	endElementNs(void * ctx, 
    					     const xmlChar * localname, 
    					     const xmlChar * prefix, 
                             const xmlChar * URI);
};


class DeleteBucketHandler  : public S3Handler
{
public:
    DeleteBucketHandler();

protected:
    enum States {
        Code        = 1,
        Message     = 2,
        RequestId   = 4,
        HostId      = 8
    };

public:
    static void startElementNs( void * ctx, 
                                const xmlChar * localname, 
                                const xmlChar * prefix, 
                                const xmlChar * URI, 
                                int nb_namespaces, 
                                const xmlChar ** namespaces, 
                                int nb_attributes, 
                                int nb_defaulted, 
                                const xmlChar ** attributes );
    
    static void	charactersSAXFunc(void * ctx, 
    					          const xmlChar * value, 
                                  int len);
    
    static void	endElementNs(void * ctx, 
    					     const xmlChar * localname, 
    					     const xmlChar * prefix, 
                             const xmlChar * URI);
};


class ListBucketHandler  : public S3Handler
{
public:
    ListBucketHandler();

protected:
    enum States {
        Code        = 1,
        Message     = 2,
        RequestId   = 4,
        HostId      = 8,
        Name        = 16,
        Prefix      = 32,
        Marker      = 64,
        Truncated   = 128,
        Contents    = 256,
        Key         = 512,
        LastModified = 1024,
        ETag         = 2048,
        Length         = 4096,
        CommonPrefixes = 8192
    };

public:
    static void startElementNs( void * ctx, 
                                const xmlChar * localname, 
                                const xmlChar * prefix, 
                                const xmlChar * URI, 
                                int nb_namespaces, 
                                const xmlChar ** namespaces, 
                                int nb_attributes, 
                                int nb_defaulted, 
                                const xmlChar ** attributes );
    
    static void	charactersSAXFunc(void * ctx, 
    					          const xmlChar * value, 
                                  int len);
    
    static void	endElementNs(void * ctx, 
    					     const xmlChar * localname, 
    					     const xmlChar * prefix, 
                             const xmlChar * URI);
};


class PutHandler  : public S3Handler
{
public:
    PutHandler();

protected:
    enum States {
        Code        = 1,
        Message     = 2,
        RequestId   = 4,
        HostId      = 8
    };

    
public:
    static void startElementNs( void * ctx, 
                                const xmlChar * localname, 
                                const xmlChar * prefix, 
                                const xmlChar * URI, 
                                int nb_namespaces, 
                                const xmlChar ** namespaces, 
                                int nb_attributes, 
                                int nb_defaulted, 
                                const xmlChar ** attributes );
    
    static void	charactersSAXFunc(void * ctx, 
    					          const xmlChar * value, 
                                  int len);
    
    static void	endElementNs(void * ctx, 
    					     const xmlChar * localname, 
    					     const xmlChar * prefix, 
                             const xmlChar * URI);
};


class GetHandler  : public S3Handler
{
public:
    GetHandler();

protected:
    enum States {
        Code         = 1,
        Message      = 2,
        RequestId    = 4,
        HostId       = 8
    };

public:
    static void startElementNs( void * ctx, 
                                const xmlChar * localname, 
                                const xmlChar * prefix, 
                                const xmlChar * URI, 
                                int nb_namespaces, 
                                const xmlChar ** namespaces, 
                                int nb_attributes, 
                                int nb_defaulted, 
                                const xmlChar ** attributes );
    
    static void	charactersSAXFunc(void * ctx, 
    					          const xmlChar * value, 
                                  int len);
    
    static void	endElementNs(void * ctx, 
    					     const xmlChar * localname, 
    					     const xmlChar * prefix, 
                             const xmlChar * URI);
};


class DeleteHandler  : public S3Handler
{
public:
    DeleteHandler();

protected:
    enum States {
        Code        = 1,
        Message     = 2,
        RequestId   = 4,
        HostId      = 8
    };

    
public:
    static void startElementNs( void * ctx, 
                                const xmlChar * localname, 
                                const xmlChar * prefix, 
                                const xmlChar * URI, 
                                int nb_namespaces, 
                                const xmlChar ** namespaces, 
                                int nb_attributes, 
                                int nb_defaulted, 
                                const xmlChar ** attributes );
    
    static void	charactersSAXFunc(void * ctx, 
    					          const xmlChar * value, 
                                  int len);
    
    static void	endElementNs(void * ctx, 
    					     const xmlChar * localname, 
    					     const xmlChar * prefix, 
                             const xmlChar * URI);
};

class HeadHandler  : public S3Handler
{
public:
    HeadHandler();

protected:
    enum States {
        Code        = 1,
        Message     = 2,
        RequestId   = 4,
        HostId      = 8
    };

    
public:
    static void startElementNs( void * ctx, 
                                const xmlChar * localname, 
                                const xmlChar * prefix, 
                                const xmlChar * URI, 
                                int nb_namespaces, 
                                const xmlChar ** namespaces, 
                                int nb_attributes, 
                                int nb_defaulted, 
                                const xmlChar ** attributes );
    
    static void	charactersSAXFunc(void * ctx, 
    					          const xmlChar * value, 
                                  int len);
    
    static void	endElementNs(void * ctx, 
    					     const xmlChar * localname, 
    					     const xmlChar * prefix, 
                             const xmlChar * URI);
};

    
} } // end namespaces


#endif
