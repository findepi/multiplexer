//
// Azouk Libraries -- Libraries and goodies created for www.azouk.com.
// Copyright (C) 2008-2009 Azouk Network Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Author:
//      Piotr Findeisen <piotr.findeisen at gmail.com>
//

#ifndef AZLIB_PREPROC_KWARGS_H
#define AZLIB_PREPROC_KWARGS_H

#define AZOUK_PP_KWARGS_PROCESS(namespace, data, tokens)    __AZOUK_PP_KWARGS_TOKEN_ ## tokens SENTINEL__, namespace, data)

# // SENTINEL__ -- stop iteration
#define __AZOUK_PP_KWARGS_TOKEN_SENTINEL__				__AZOUK_PP_KWARGS_TOKEN_SENTINEL_I__ ( + 
#define __AZOUK_PP_KWARGS_TOKEN_SENTINEL_I__(x, namespace, data)

# // DEFAULT (no-op) token
#define __AZOUK_PP_KWARGS_TOKEN_DEFAULT					__AZOUK_PP_KWARGS_TOKEN_DEFAULT_I (
#define __AZOUK_PP_KWARGS_TOKEN_DEFAULT_I(tokens, namespace, data)	__AZOUK_PP_KWARGS_TOKEN_ ## tokens , namespace, data)

# // LEVEL(level)
#define __AZOUK_PP_KWARGS_TOKEN_LEVEL(level)				__AZOUK_PP_KWARGS_TOKEN_LEVEL_I ( level ,
#define __AZOUK_PP_KWARGS_TOKEN_LEVEL_I(level, tokens, namespace, data)	namespace ## LEVEL(level, data) \
									__AZOUK_PP_KWARGS_TOKEN_ ## tokens, namespace, data)
# // VERBOSITY(verbosity)
#define __AZOUK_PP_KWARGS_TOKEN_VERBOSITY(verbosity)				__AZOUK_PP_KWARGS_TOKEN_VERBOSITY_I ( verbosity ,
#define __AZOUK_PP_KWARGS_TOKEN_VERBOSITY_I(verbosity, tokens, namespace, data)	namespace ## VERBOSITY(verbosity, data) \
									__AZOUK_PP_KWARGS_TOKEN_ ## tokens, namespace, data)
# // FLOW(flow)
#define __AZOUK_PP_KWARGS_TOKEN_FLOW(flow)				__AZOUK_PP_KWARGS_TOKEN_FLOW_I ( flow ,
#define __AZOUK_PP_KWARGS_TOKEN_FLOW_I(flow, tokens, namespace, data)	namespace ## FLOW(flow, data) \
									__AZOUK_PP_KWARGS_TOKEN_ ## tokens, namespace, data)
# // TEXT(text)
#define __AZOUK_PP_KWARGS_TOKEN_TEXT(text)				__AZOUK_PP_KWARGS_TOKEN_TEXT_I ( text ,
#define __AZOUK_PP_KWARGS_TOKEN_TEXT_I(text, tokens, namespace, data)	namespace ## TEXT(text, data) \
									__AZOUK_PP_KWARGS_TOKEN_ ## tokens, namespace, data)
// CTX(context)
#define __AZOUK_PP_KWARGS_TOKEN_CTX(text)				__AZOUK_PP_KWARGS_TOKEN_CTX_I ( text ,
#define __AZOUK_PP_KWARGS_TOKEN_CTX_I(text, tokens, namespace, data)	namespace ## CTX(text, data) \
									__AZOUK_PP_KWARGS_TOKEN_ ## tokens, namespace, data)
// CONTEXT(context)
#define __AZOUK_PP_KWARGS_TOKEN_CONTEXT(text)				    __AZOUK_PP_KWARGS_TOKEN_CONTEXT_I ( text ,
#define __AZOUK_PP_KWARGS_TOKEN_CONTEXT_I(text, tokens, namespace, data)    namespace ## CONTEXT(text, data) \
									    __AZOUK_PP_KWARGS_TOKEN_ ## tokens, namespace, data)

# // DATA(type_id, class, setters)
#define __AZOUK_PP_KWARGS_TOKEN_DATA(type_id, type, setters)		__AZOUK_PP_KWARGS_TOKEN_DATA_I ( type_id, type, setters ,
#define __AZOUK_PP_KWARGS_TOKEN_DATA_I(type_id, type, setters, tokens, namespace, data) \
									namespace ## DATA(type_id, type, setters, data) \
									__AZOUK_PP_KWARGS_TOKEN_ ## tokens, namespace, data)

# // MUSTLOG token
#define __AZOUK_PP_KWARGS_TOKEN_MUSTLOG					__AZOUK_PP_KWARGS_TOKEN_MUSTLOG_I (
#define __AZOUK_PP_KWARGS_TOKEN_MUSTLOG_I(tokens, namespace, data)	namespace ## MUSTLOG(data) \
									__AZOUK_PP_KWARGS_TOKEN_ ## tokens , namespace, data)

# // SKIPFILEIF(b)
#define __AZOUK_PP_KWARGS_TOKEN_SKIPFILEIF(b)				    __AZOUK_PP_KWARGS_TOKEN_SKIPFILEIF_I ( b ,
#define __AZOUK_PP_KWARGS_TOKEN_SKIPFILEIF_I(b, tokens, namespace, data)    namespace ## SKIPFILEIF(b, data) \
									    __AZOUK_PP_KWARGS_TOKEN_ ## tokens, namespace, data)

#endif
