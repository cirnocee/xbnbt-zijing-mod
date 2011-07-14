/***
*
* BNBT Beta 8.0 - A C++ BitTorrent Tracker
* Copyright (C) 2003-2004 Trevor Hogan
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
***/

// =Xotic= Modified Source File

#if defined ( WIN32 )
 #include <sys/stat.h>
 #include <time.h>
#else
 #include <dirent.h>
//  #include <fcntl.h>
 #include <sys/stat.h>
 #include <sys/time.h>
#endif

#include "bnbt.h"
#include "bnbt_mysql.h"
#include "atom.h"
#include "bencode.h"
#include "config.h"
#include "html.h"
#include "md5.h"
#include "server.h"
#include "sort.h"
#include "tracker.h"
#include "util.h"

void CTracker :: serverResponseQuery( struct request_t *pRequest, struct response_t *pResponse )
{
	// Verify that the IP is permitted to access the tracker
	if( m_ucIPBanMode != 0 )
	{
		// Set the start time
		const struct bnbttv btv( UTIL_CurrentTime( ) );

		if( IsIPBanned( pRequest, pResponse, btv, "IP Banned", string( CSS_INDEX ), NOT_INDEX ) )
			return;
	}

	pResponse->strCode = "200 " + gmapLANG_CFG["server_response_200"];

	pResponse->mapHeaders.insert( pair<string, string>( "Content-Type", gmapMime[".xml"] ) );

	pResponse->strContent += "<?xml version=\"1.0\" encoding=\"" + gstrCharSet + "\" ?>\n";

	pResponse->strContent += "<result>\n";

	pResponse->strContent += "<description>" + gmapLANG_CFG["query_description"] + "</description>\n";

	const string cstrType( pRequest->mapParams["type"] );
	const string cstrID( pRequest->mapParams["id"] );
	const string cstrAction( pRequest->mapParams["action"] );

	if( !cstrType.empty( ) )
		pResponse->strContent += "<title>" + gmapLANG_CFG["query_"+cstrType] + "</title>\n";

	if( cstrType == "thanks" )
	{
		if( !pRequest->user.strUID.empty( ) && ( pRequest->user.ucAccess & m_ucAccessViewStats ) && !cstrID.empty( ) && !cstrAction.empty( ) )
		{
			pResponse->strContent += "<action>" + gmapLANG_CFG["query_"+cstrType+"_"+cstrAction] + "</action>\n";

			if( cstrAction == "do" )
			{
				CMySQLQuery *pQuery = new CMySQLQuery( "SELECT buploaderid,bthanks FROM allowed WHERE bid=" + cstrID );

				vector<string> vecQuery;

				vecQuery.reserve(2);

				vecQuery = pQuery->nextRow( );
	
				delete pQuery;
	
				if( vecQuery.size( ) == 2 && vecQuery[0] != pRequest->user.strUID ) 
				{
					unsigned int uiThanksBonus = CFG_GetInt( "bnbt_thanks_bonus", 3 );

					CMySQLQuery *pQueryThanks = new CMySQLQuery( "SELECT bid,bthankerid FROM thanks WHERE bid=" + cstrID + " AND bthankerid=" + pRequest->user.strUID );

					vector<string> vecQueryThanks;

					vecQueryThanks.reserve(2);

					vecQueryThanks = pQueryThanks->nextRow( );
	
					delete pQueryThanks;
		
					if( vecQueryThanks.size( ) == 0 )
					{
						CMySQLQuery mq01( "INSERT INTO thanks (bid,bthankerid,bthanker,bthanktime) VALUES(" + cstrID + "," + pRequest->user.strUID + ",\'" + UTIL_StringToMySQL( pRequest->user.strLogin ) + "\',NOW())" );
//						m_pCache->setUserData( vecQuery[0], 0, 0, uiThanksBonus * 100 );
						CMySQLQuery mq02( "UPDATE allowed SET bthanks=bthanks+1 WHERE bid=" + cstrID );
						CMySQLQuery mq03( "UPDATE users SET bbonus=bbonus+" + CAtomInt( uiThanksBonus * 100 ).toString( ) + " WHERE buid=" + vecQuery[0] );
						m_pCache->setThanks( cstrID, SET_THANKS_ADD );

						pResponse->strContent += "<status>" + gmapLANG_CFG["query_thanks_successful"] + "</status>\n";
						pResponse->strContent += "<code>1</code>\n";
					}
					else if( vecQueryThanks.size( ) == 2 )
					{
						pResponse->strContent += "<status>" + gmapLANG_CFG["query_thanks_again"] + "</status>\n";
						pResponse->strContent += "<code>2</code>\n";
					}
					pResponse->strContent += "<value>" + vecQuery[1] + "</value>\n";
				}
				else
				{
					pResponse->strContent += "<status>" + gmapLANG_CFG["query_thanks_failed"] + "</status>\n";
					pResponse->strContent += "<code>0</code>\n";
				}
			}
		}
		else
		{
			pResponse->strContent += "<status>" + gmapLANG_CFG["query_failed"] + "</status>\n";
			pResponse->strContent += "<code>-1</code>\n";
		}
	}
	else if( cstrType == "bookmark" )
	{
		if( !pRequest->user.strUID.empty( ) && ( pRequest->user.ucAccess & m_ucAccessBookmark ) && !cstrID.empty( ) && !cstrAction.empty( ) )
		{
			pResponse->strContent += "<action>" + gmapLANG_CFG["query_"+cstrType+"_"+cstrAction] + "</action>\n";

			if( cstrAction == "add" )
			{
				CMySQLQuery *pQuery = new CMySQLQuery( "SELECT bid FROM allowed WHERE bid=" + cstrID );
		
				vector<string> vecQuery;
		
				vecQuery.reserve(1);

				vecQuery = pQuery->nextRow( );
			
				delete pQuery;
			
				if( vecQuery.size( ) == 1 && !vecQuery[0].empty( ) )
				{
					CMySQLQuery *pQueryBookmark = new CMySQLQuery( "SELECT buid,bid FROM bookmarks WHERE buid=" + pRequest->user.strUID + " AND bid=" + cstrID );
		
					vector<string> vecQueryBookmark;
		
					vecQueryBookmark.reserve(2);

					vecQueryBookmark = pQueryBookmark->nextRow( );
				
					delete pQueryBookmark;
				
					if( vecQueryBookmark.size( ) == 0 )
					{
						CMySQLQuery mq01( "INSERT INTO bookmarks (buid,bid) VALUES(" + pRequest->user.strUID + "," + cstrID + ")" );
						pResponse->strContent += "<status>" + gmapLANG_CFG["query_bookmark_successful"] + "</status>\n";
						pResponse->strContent += "<code>1</code>\n";
					}
					else if( vecQueryBookmark.size( ) == 2 )
					{
						pResponse->strContent += "<status>" + gmapLANG_CFG["query_bookmark_exist"] + "</status>\n";
						pResponse->strContent += "<code>2</code>\n";
					}
				}
				else
				{
					pResponse->strContent += "<status>" + gmapLANG_CFG["query_bookmark_failed"] + "</status>\n";
					pResponse->strContent += "<code>0</code>\n";
				}
			}
			else if( cstrAction == "remove" )
			{
				CMySQLQuery *pQuery = new CMySQLQuery( "SELECT bid FROM allowed WHERE bid=" + cstrID );
		
				vector<string> vecQuery;
		
				vecQuery.reserve(1);

				vecQuery = pQuery->nextRow( );
			
				delete pQuery;
			
				if( vecQuery.size( ) == 1 && !vecQuery[0].empty( ) )
				{
					CMySQLQuery *pQueryBookmark = new CMySQLQuery( "SELECT buid,bid FROM bookmarks WHERE buid=" + pRequest->user.strUID + " AND bid=" + cstrID );
		
					vector<string> vecQueryBookmark;
		
					vecQueryBookmark.reserve(2);

					vecQueryBookmark = pQueryBookmark->nextRow( );
				
					delete pQueryBookmark;
				
					if( vecQueryBookmark.size( ) == 2 )
					{
						CMySQLQuery mq01( "DELETE FROM bookmarks WHERE buid=" + pRequest->user.strUID + " AND bid=" + cstrID );
						pResponse->strContent += "<status>" + gmapLANG_CFG["query_bookmark_successful"] + "</status>\n";
						pResponse->strContent += "<code>1</code>\n";
					}
					else if( vecQueryBookmark.size( ) == 0 )
					{
						pResponse->strContent += "<status>" + gmapLANG_CFG["query_bookmark_nonexist"] + "</status>\n";
						pResponse->strContent += "<code>3</code>\n";
					}
				}
				else
				{
					pResponse->strContent += "<status>" + gmapLANG_CFG["query_bookmark_failed"] + "</status>\n";
					pResponse->strContent += "<code>0</code>\n";
				}
			}
			else if( cstrAction == "share" )
			{
				const string cstrSet( pRequest->mapParams["set"] );

				CMySQLQuery *pQuery = new CMySQLQuery( "SELECT bid FROM allowed WHERE bid=" + cstrID );
		
				vector<string> vecQuery;
		
				vecQuery.reserve(1);

				vecQuery = pQuery->nextRow( );
			
				delete pQuery;
			
				if( vecQuery.size( ) == 1 && !vecQuery[0].empty( ) )
				{
					CMySQLQuery *pQueryBookmark = new CMySQLQuery( "SELECT buid,bid,bshare FROM bookmarks WHERE buid=" + pRequest->user.strUID + " AND bid=" + cstrID );
		
					vector<string> vecQueryBookmark;
		
					vecQueryBookmark.reserve(3);

					vecQueryBookmark = pQueryBookmark->nextRow( );
				
					delete pQueryBookmark;
				
					if( vecQueryBookmark.size( ) == 3 )
					{
						if( ( cstrSet == "0" || cstrSet == "1" ) && cstrSet != vecQueryBookmark[2] )
						{
							CMySQLQuery mq01( "UPDATE bookmarks SET bshare=" + cstrSet + " WHERE buid=" + pRequest->user.strUID + " AND bid=" + cstrID );
							pResponse->strContent += "<status>" + gmapLANG_CFG["query_bookmark_successful"] + "</status>\n";
							pResponse->strContent += "<code>1</code>\n";
						}
						else
						{
							pResponse->strContent += "<status>" + gmapLANG_CFG["query_bookmark_sameshare"] + "</status>\n";
							pResponse->strContent += "<code>4</code>\n";
						}
					}
					else if( vecQueryBookmark.size( ) == 0 )
					{
						pResponse->strContent += "<status>" + gmapLANG_CFG["query_bookmark_nonexist"] + "</status>\n";
						pResponse->strContent += "<code>3</code>\n";
					}
				}
				else
				{
					pResponse->strContent += "<status>" + gmapLANG_CFG["query_bookmark_failed"] + "</status>\n";
					pResponse->strContent += "<code>0</code>\n";
				}
			}
		}
		else
		{
			pResponse->strContent += "<status>" + gmapLANG_CFG["query_failed"] + "</status>\n";
			pResponse->strContent += "<code>-1</code>\n";
		}
	}
	else if( cstrType == "request" )
	{
		if( !pRequest->user.strUID.empty( ) && ( pRequest->user.ucAccess & m_ucAccessReq ) && !cstrID.empty( ) && !cstrAction.empty( ) )
		{
			pResponse->strContent += "<action>" + gmapLANG_CFG["query_"+cstrType+"_"+cstrAction] + "</action>\n";

			if( cstrAction == "add" )
			{
				CMySQLQuery *pQuery = new CMySQLQuery( "SELECT bid,btitle,buploaderid,breq,bnodownload,bseeders,bpost FROM allowed WHERE bid=" + cstrID );
			
				vector<string> vecQuery;
			
				vecQuery.reserve(7);

				vecQuery = pQuery->nextRow( );
				
				delete pQuery;
				
				if( vecQuery.size( ) == 7 && !vecQuery[3].empty( ) && vecQuery[2] != pRequest->user.strUID && vecQuery[4] == "0" && vecQuery[5] == "0" && vecQuery[6] == "0" )
				{
					if( vecQuery[3] == "0" )
					{
						m_pCache->setStatus( cstrID, SET_STATUS_REQ );
						
						CMySQLQuery mq01( "UPDATE allowed SET breq=1 WHERE bid=" + cstrID );
						pResponse->strContent += "<status>" + gmapLANG_CFG["query_request_successful"] + "</status>\n";
						pResponse->strContent += "<code>1</code>\n";

						string cstrName = vecQuery[1];
						string cstrUploaderID = vecQuery[2];

						CMySQLQuery *pQueryUsers = new CMySQLQuery( "SELECT buid FROM users WHERE buid=" + cstrUploaderID );

						vector<string> vecQueryUsers;
					
						vecQueryUsers.reserve(1);

						vecQueryUsers = pQueryUsers->nextRow( );
						
						delete pQueryUsers;

						if( vecQueryUsers.size( ) == 1 && !vecQueryUsers[0].empty( ) )
						{
							if( !pRequest->user.strUID.empty( ) )
							{
								string strTitle = gmapLANG_CFG["section_reqseeders"];
								string strMessage = UTIL_Xsprintf( gmapLANG_CFG["req_seeders_message"].c_str( ), pRequest->user.strLogin.c_str( ), string( RESPONSE_STR_STATS_HTML + "?id=" + cstrID ).c_str( ), cstrName.c_str( ) );
								
								sendMessage( pRequest->user.strLogin, pRequest->user.strUID, cstrUploaderID, pRequest->strIP, strTitle, strMessage );
							}
						}
					}
					else if( vecQuery[3] == "1" )
					{
						pResponse->strContent += "<status>" + gmapLANG_CFG["query_request_again"] + "</status>\n";
						pResponse->strContent += "<code>2</code>\n";
					}
				}
				else
				{
					pResponse->strContent += "<status>" + gmapLANG_CFG["query_request_failed"] + "</status>\n";
					pResponse->strContent += "<code>0</code>\n";
				}
			}
			else if( cstrAction == "remove" )
			{
				CMySQLQuery *pQuery = new CMySQLQuery( "SELECT bid,breq,bseeders FROM allowed WHERE bid=" + cstrID );
			
				vector<string> vecQuery;
			
				vecQuery.reserve(3);

				vecQuery = pQuery->nextRow( );
				
				delete pQuery;
				
				if( vecQuery.size( ) == 3 && !vecQuery[1].empty( ) && vecQuery[2] != "0" )
				{
					if( vecQuery[1] == "1" )
					{
						m_pCache->setStatus( cstrID, SET_STATUS_NOREQ );
					
						CMySQLQuery mq01( "UPDATE allowed SET breq=0 WHERE bid=" + cstrID );
						pResponse->strContent += "<status>" + gmapLANG_CFG["query_request_successful"] + "</status>\n";
						pResponse->strContent += "<code>1</code>\n";
					}
					else if( vecQuery[1] == "0" )
					{
						pResponse->strContent += "<status>" + gmapLANG_CFG["query_request_again"] + "</status>\n";
						pResponse->strContent += "<code>2</code>\n";
					}
				}
				else
				{
					pResponse->strContent += "<status>" + gmapLANG_CFG["query_request_failed"] + "</status>\n";
					pResponse->strContent += "<code>0</code>\n";
				}
			}
		}
		else
		{
			pResponse->strContent += "<status>" + gmapLANG_CFG["query_failed"] + "</status>\n";
			pResponse->strContent += "<code>-1</code>\n";
		}
	}

	pResponse->strContent += "</result>\n";
}