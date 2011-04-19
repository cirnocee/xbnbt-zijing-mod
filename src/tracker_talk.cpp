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
 #include <time.h>
#else
 #include <sys/time.h>
#endif

#include "bnbt.h"
#include "bnbt_mysql.h"
#include "atom.h"
#include "config.h"
#include "html.h"
#include "tracker.h"
#include "util.h"

void CTracker :: serverResponseTalkGET( struct request_t *pRequest, struct response_t *pResponse )
{
	// Set the start time
	const struct bnbttv btv( UTIL_CurrentTime( ) );

	// Verify that the IP is permitted to access the tracker
	if( m_ucIPBanMode != 0 )
		if( IsIPBanned( pRequest, pResponse, btv, gmapLANG_CFG["talk_page"], string( CSS_TALK ), NOT_INDEX ) )
			return;

	if( !m_bAllowComments || pRequest->user.strUID.empty( ) )
	{
		// Output common HTML head
		HTML_Common_Begin(  pRequest, pResponse, gmapLANG_CFG["talk_page"], string( CSS_TALK ), string( ), NOT_INDEX, CODE_401 );

		// Output common HTML tail
		HTML_Common_End( pRequest, pResponse, btv, NOT_INDEX, string( CSS_TALK ) );

		return;
	}

	if( pRequest->user.ucAccess & m_ucAccessView )
	{
		// Output common HTML head
		HTML_Common_Begin(  pRequest, pResponse, gmapLANG_CFG["talk_page"], string( CSS_TALK ), string( ), NOT_INDEX, CODE_200 );
		
//		pResponse->strContent += "<META http-equiv=\"refresh\" content=\"30\" URL=\"" + RESPONSE_STR_TALK_HTML + "\">\n";

		// assorted scripts (thanks SA)
		pResponse->strContent += "<script type=\"text/javascript\">\n";
		pResponse->strContent += "<!--\n";

		// validate
		pResponse->strContent += "function validate( theform ) {\n";
		pResponse->strContent += "if( theform.talk.value == \"\" ) {\n";
		pResponse->strContent += "  alert( \"" + gmapLANG_CFG["js_fill_fields"] + "\" );\n";
		pResponse->strContent += "  return false; }\n";
		pResponse->strContent += "if( theform.talk.value.getBytes() > " + CAtomInt( m_uiTalkLength ).toString( ) + " ) {\n";
		pResponse->strContent += "  alert( \"" + gmapLANG_CFG["js_too_long"] + "\\n" + m_strJSTalkLength + "\\n" + m_strJSReduce + "\" );\n";
		pResponse->strContent += "  return false; }\n";
		pResponse->strContent += "else { return true; }\n";
		pResponse->strContent += "}\n\n";
		
		pResponse->strContent += "String.prototype.getBytes = function() {\n";
		pResponse->strContent += "  return this.length;\n";
//		pResponse->strContent += "  var cArr = this.match(/[^\\x00-\\xff]/ig);\n";
//		pResponse->strContent += "  return this.length + (cArr == null ? 0 : cArr.length);\n";
		pResponse->strContent += "}\n\n";
		
		pResponse->strContent += "String.prototype.stripX = function() {\n";
		pResponse->strContent += "  var str = this;\n";
		pResponse->strContent += "  str = str.replace(/\\&nbsp;/g,' ');\n";
		pResponse->strContent += "  str = str.replace(/\\&quot;/g,'\"');\n";
		pResponse->strContent += "  str = str.replace(/\\&amp;/g,'&');\n";
		pResponse->strContent += "  str = str.replace(/\\&gt;/g,'>');\n";
		pResponse->strContent += "  str = str.replace(/\\&lt;/g,'<');\n";
		pResponse->strContent += "  return str;\n";
		pResponse->strContent += "}\n\n";
		
//		pResponse->strContent += "var textarea = document.getElementById ('talkarea');\n";
//		pResponse->strContent += "if(/msie/i.test(navigator.userAgent)) {\n";
//		pResponse->strContent += "  textarea.onPropertyChange=charleft; }\n";
//		pResponse->strContent += "else {\n";
//		pResponse->strContent += "  if(textarea.addEventListener)\n";
//		pResponse->strContent += "    textarea.addEventListener(\"textInput\",charleft,false); }\n";

		// checklength
		pResponse->strContent += "function checklength( theform ) {\n";
		pResponse->strContent += "  alert( \"" + UTIL_Xsprintf( gmapLANG_CFG["js_talk_length"].c_str( ), "\" + theform.talk.value.getBytes() + \"" ) + "\" );\n";
		pResponse->strContent += "}\n\n";
		
		// key post
		pResponse->strContent += "function keypost( evt, id ) {\n";
		pResponse->strContent += "  var key=evt.keyCode;\n";
		pResponse->strContent += "  if( (key==13) && (evt.ctrlKey) )\n";
		pResponse->strContent += "    document.getElementById( id ).click();\n";
		pResponse->strContent += "}\n\n";
		
		// charleft
		pResponse->strContent += "function charleft( theform, id, submitid ) {\n";
		pResponse->strContent += "  if( " + CAtomInt( m_uiTalkLength ).toString( ) + "-theform.talk.value.getBytes()>=0 ) {\n";
		pResponse->strContent += "    document.getElementById( submitid ).disabled = false;\n";
		pResponse->strContent += "    document.getElementById( id ).innerHTML = \"" + UTIL_Xsprintf( gmapLANG_CFG["talk_left"].c_str( ), string("\" + (" +  CAtomInt( m_uiTalkLength ).toString( ) + "-theform.talk.value.getBytes()) + \"" ).c_str( ) ) + "\";\n";
		pResponse->strContent += "    document.getElementById( id ).style.color = \"black\"; }\n";
		pResponse->strContent += "  else {\n";
		pResponse->strContent += "    document.getElementById( submitid ).disabled = true;\n";
		pResponse->strContent += "    document.getElementById( id ).innerHTML = \"" + UTIL_Xsprintf( gmapLANG_CFG["talk_above"].c_str( ), string("\" + (theform.talk.value.getBytes()-" +  CAtomInt( m_uiTalkLength ).toString( ) + ") + \"" ).c_str( ) ) + "\";\n";
		pResponse->strContent += "    document.getElementById( id ).style.color = \"red\"; }\n";
		pResponse->strContent += "}\n\n";
		
		pResponse->strContent += "if (window.XMLHttpRequest)\n";
		pResponse->strContent += "{// code for IE7+, Firefox, Chrome, Opera, Safari\n";
		pResponse->strContent += "  xmlhttp=new XMLHttpRequest(); }\n";
		pResponse->strContent += "else\n";
		pResponse->strContent += "{// code for IE6, IE5\n";
		pResponse->strContent += "  xmlhttp=new ActiveXObject(\"Microsoft.XMLHTTP\"); }\n\n";
		
		pResponse->strContent += "var posted;\n\n";
		
		// post
		pResponse->strContent += "function post(formId,textareaId,submitId) {\n";
		pResponse->strContent += "  var the_timeout;\n";
		pResponse->strContent += "  if (validate( document.postatalk )) {\n";
		pResponse->strContent += "    var postForm = document.getElementById( formId );\n";
		pResponse->strContent += "    var postTextarea = document.getElementById( textareaId );\n";
		pResponse->strContent += "    var postSubmit = document.getElementById( submitId );\n";
		pResponse->strContent += "    var post_data = '';\n";
		pResponse->strContent += "    var textData = postTextarea.value;\n";
		pResponse->strContent += "    var i = 0;\n";
		pResponse->strContent += "    for(i=0;i<textData.length;i++) {\n";
		pResponse->strContent += "      if(textData.charAt(i) != ' ')\n";
		pResponse->strContent += "        break;\n";
		pResponse->strContent += "    }\n";
		pResponse->strContent += "    textData = textData.substring(i,textData.length);\n";
		pResponse->strContent += "    for(i=textData.length;i>0;i--) {\n";
		pResponse->strContent += "      if(textData.charAt(i-1) != ' ')\n";
		pResponse->strContent += "        break;\n";
		pResponse->strContent += "    }\n";
		pResponse->strContent += "    textData = textData.substring(0,i);\n";
		pResponse->strContent += "    postTextarea.value = textData;\n";
		pResponse->strContent += "    for (var i = 0; i < postForm.elements.length; i++) {\n";
		pResponse->strContent += "      if ( (postForm.elements[i].name != 'submit_talk_button') && !(postForm.elements[i].name == 'show' && postForm.elements[i].value == 'mentions') && postForm.elements[i].name != 'uid' ) {\n";
		pResponse->strContent += "        if( post_data != '' )\n";
		pResponse->strContent += "          post_data = post_data + '&';\n";
		pResponse->strContent += "        post_data = post_data + postForm.elements[i].name + '=' + encodeURIComponent(postForm.elements[i].value); }\n";
		pResponse->strContent += "    }\n";
		pResponse->strContent += "    xmlhttp.onreadystatechange=function() {\n";
		pResponse->strContent += "      if (xmlhttp.readyState==4 && xmlhttp.status==200) {\n";
		pResponse->strContent += "          clearTimeout( the_timeout );\n";
		pResponse->strContent += "          postTextarea.value = '';\n";
		pResponse->strContent += "          setValue( 'inputReply', '' );\n";
		pResponse->strContent += "          setValue( 'inputReplyTo', '' );\n";
		pResponse->strContent += "          setValue( 'inputRT', '' );\n";
		pResponse->strContent += "          setValue( 'inputRTTo', '' );\n";
		pResponse->strContent += "          charleft(document.postatalk,'talk_left','submit_talk');\n";
//		pResponse->strContent += "          window.scrollTo(0,0);\n";
//		pResponse->strContent += "          window.location.hash = \"#\";\n";
		pResponse->strContent += "          document.postatalk.talk.focus();\n";
		pResponse->strContent += "          postSubmit.disabled = false;\n";
		pResponse->strContent += "          load('div','divTalk',postForm.action+'?'+post_data); }\n";
		pResponse->strContent += "    }\n";
		pResponse->strContent += "    xmlhttp.open(\"POST\",postForm.action,true);\n";
//		pResponse->strContent += "    xmlhttp.setRequestHeader(\"Content-Length\", post_data.length);\n";
		pResponse->strContent += "    xmlhttp.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");\n";
//		pResponse->strContent += "    xmlhttp.setRequestHeader(\"Connection\", \"close\");\n";
		pResponse->strContent += "    xmlhttp.send(post_data);\n";
		pResponse->strContent += "    postSubmit.disabled = true;\n";
		pResponse->strContent += "    posted = false;\n";
		pResponse->strContent += "    var funcTimeout = call_timeout('submit_talk');\n";
		pResponse->strContent += "    the_timeout= setTimeout( funcTimeout, 15000 );\n";
		pResponse->strContent += "  }\n";
		pResponse->strContent += "}\n\n";
		
		// call_timeout
		pResponse->strContent += "function call_timeout(id) {\n";
		pResponse->strContent += "  return (function() {\n";
		pResponse->strContent += "    timeout(id); })\n";
		pResponse->strContent += "}\n\n";
		
		// post timeout
		pResponse->strContent += "function timeout(id) {\n";
		pResponse->strContent += "  var element = document.getElementById( id );\n";
		pResponse->strContent += "  if( element.disabled == true ) {\n";
		pResponse->strContent += "    alert('" + gmapLANG_CFG["talk_timeout"] + "');\n";
//		pResponse->strContent += "    clearAll();\n";
		pResponse->strContent += "    element.disabled = false; }\n";
		pResponse->strContent += "}\n\n";
		
		// reply
		pResponse->strContent += "function reply(replyId,replytoId,formId,textareaId,inputId) {\n";
		pResponse->strContent += "  var replySpan = document.getElementById( 'reply'+replyId );\n";
		pResponse->strContent += "  var replyData = replySpan.innerHTML;\n";
		pResponse->strContent += "  var textarea = document.getElementById( textareaId );\n";
		pResponse->strContent += "  var textData = textarea.value;\n";
		pResponse->strContent += "  var replyInput = document.getElementById( inputId );\n";
		pResponse->strContent += "  var replytoInput = document.getElementById( inputId+'To' );\n";
		pResponse->strContent += "  var i = 0;\n";
		pResponse->strContent += "  for(i=0;i<textData.length;i++) {\n";
		pResponse->strContent += "    if(textData.charAt(i) != ' ')\n";
		pResponse->strContent += "      break;\n";
		pResponse->strContent += "  }\n";
		pResponse->strContent += "  textData = textData.substring(i,textData.length);\n";
		pResponse->strContent += "  for(i=textData.length;i>0;i--) {\n";
		pResponse->strContent += "    if(textData.charAt(i-1) != ' ')\n";
		pResponse->strContent += "      break;\n";
		pResponse->strContent += "  }\n";
		pResponse->strContent += "  window.scrollTo(0,0);\n";
		pResponse->strContent += "  document.postatalk.talk.focus();\n";
		pResponse->strContent += "  textData = textData.substring(0,i) + ' ';\n";
		pResponse->strContent += "  var start = textData.indexOf( replyData );\n";
		pResponse->strContent += "  var done = false;\n";
		pResponse->strContent += "  if( start != -1 ) {\n";
		pResponse->strContent += "    while( start != -1 ) {\n";
		pResponse->strContent += "      if( (start > 0 && textData.charAt( start-1 ) == ' ') || start == 0 ) {\n";
		pResponse->strContent += "        textarea.value = replyData + textData.substring(0,start) + textData.substring(start+replyData.length,textData.length);\n";
		pResponse->strContent += "        done = true;\n";
		pResponse->strContent += "        break; }\n";
		pResponse->strContent += "      start = textData.indexOf( replyData, start + replyData.length );\n";
		pResponse->strContent += "    }\n";
		pResponse->strContent += "  }\n";
		pResponse->strContent += "  if( done == false ) {\n";
		pResponse->strContent += "    if( textData != ' ' )\n";
		pResponse->strContent += "      textarea.value = replyData + textData;\n";
		pResponse->strContent += "    else\n";
		pResponse->strContent += "      textarea.value = replyData; }\n";
		pResponse->strContent += "  charleft(document.postatalk,'talk_left','submit_talk');\n";
		pResponse->strContent += "  replyInput.value = replyId;\n";
		pResponse->strContent += "  replytoInput.value = replytoId;\n";
//		pResponse->strContent += "  window.location.hash = \"#\";\n";
//		pResponse->strContent += "  else\n";
//		pResponse->strContent += "      textarea.value = textData;
		pResponse->strContent += "}\n\n";
//		pResponse->strContent += "function reply(replyId,formId,formName) {\n";
//		pResponse->strContent += "  var replyForm = document.getElementById( formId );\n";
//		pResponse->strContent += "  var get_data = '?reply=' + replyId;\n";
//		pResponse->strContent += "  for (var i = 0; i < replyForm.elements.length; i++) {\n";
//		pResponse->strContent += "    if (replyForm.elements[i].name == 'show')\n";
//		pResponse->strContent += "      get_data = get_data + '&' + replyForm.elements[i].name + '=' + replyForm.elements[i].value;\n";
//		pResponse->strContent += "  }\n";
//		pResponse->strContent += "  xmlhttp.onreadystatechange=function() {\n";
//		pResponse->strContent += "    if (xmlhttp.readyState==4 && xmlhttp.status==200) {\n";
//		pResponse->strContent += "      var e = document.createElement('div');\n";
//		pResponse->strContent += "      e.innerHTML = xmlhttp.responseText;\n";
//		pResponse->strContent += "      var forms = e.getElementsByTagName('form');\n";
//		pResponse->strContent += "      for (var i = 0; i < forms.length; i++) {\n";
//		pResponse->strContent += "        if (forms[i].name == formName) {\n";
//		pResponse->strContent += "          replyForm.parentNode.innerHTML = forms[i].parentNode.innerHTML;\n";
//		pResponse->strContent += "          charleft(document.postatalk,'talk_left','submit_talk');\n";
//		pResponse->strContent += "          window.location.hash = \"#\";\n";
//		pResponse->strContent += "          document.postatalk.talk.focus();\n";
//		pResponse->strContent += "          break; }\n";
//		pResponse->strContent += "      }\n";
//		pResponse->strContent += "    }\n";
//		pResponse->strContent += "  }\n";
//		pResponse->strContent += "  xmlhttp.open(\"GET\",replyForm.action + get_data,true);\n";
//		pResponse->strContent += "  xmlhttp.send();\n";
//		pResponse->strContent += "}\n\n";
		
		// rt
		pResponse->strContent += "function rt(rtId,rttoId,formId,textareaId,inputreplyId,inputId) {\n";
		pResponse->strContent += "  var rtSpan = document.getElementById( 'rt'+rtId );\n";
		pResponse->strContent += "  var rtData = rtSpan.innerHTML;\n";
		pResponse->strContent += "  var textarea = document.getElementById( textareaId );\n";
		pResponse->strContent += "  var replyInput = document.getElementById( inputreplyId );\n";
		pResponse->strContent += "  var replytoInput = document.getElementById( inputreplyId+'To' );\n";
		pResponse->strContent += "  var rtInput = document.getElementById( inputId );\n";
		pResponse->strContent += "  var rttoInput = document.getElementById( inputId+'To' );\n";
		pResponse->strContent += "  window.scrollTo(0,0);\n";
		pResponse->strContent += "  document.postatalk.talk.focus();\n";
//		pResponse->strContent += "  var textData = textarea.value;\n";
		pResponse->strContent += "  textarea.value = rtData.stripX( );\n";
		pResponse->strContent += "  charleft(document.postatalk,'talk_left','submit_talk');\n";
		pResponse->strContent += "  replyInput.value = '';\n";
		pResponse->strContent += "  replytoInput.value = '';\n";
		pResponse->strContent += "  rtInput.value = rtId;\n";
		pResponse->strContent += "  rttoInput.value = rttoId;\n";
//		pResponse->strContent += "  window.location.hash = \"#\";\n";
		pResponse->strContent += "}\n\n";
//		pResponse->strContent += "function rt(rtId,formId,formName) {\n";
//		pResponse->strContent += "  var rtForm = document.getElementById( formId );\n";
//		pResponse->strContent += "  var get_data = '?rt=' + rtId;\n";
//		pResponse->strContent += "  for (var i = 0; i < rtForm.elements.length; i++) {\n";
//		pResponse->strContent += "    if (rtForm.elements[i].name == 'show')\n";
//		pResponse->strContent += "      get_data = get_data + '&' + rtForm.elements[i].name + '=' + rtForm.elements[i].value;\n";
//		pResponse->strContent += "  }\n";
//		pResponse->strContent += "  xmlhttp.onreadystatechange=function() {\n";
//		pResponse->strContent += "    if (xmlhttp.readyState==4 && xmlhttp.status==200) {\n";
//		pResponse->strContent += "      var e = document.createElement('div');\n";
//		pResponse->strContent += "      e.innerHTML = xmlhttp.responseText;\n";
//		pResponse->strContent += "      var forms = e.getElementsByTagName('form');\n";
//		pResponse->strContent += "      for (var i = 0; i < forms.length; i++) {\n";
//		pResponse->strContent += "        if (forms[i].name == formName) {\n";
//		pResponse->strContent += "          rtForm.parentNode.innerHTML = forms[i].parentNode.innerHTML;\n";
//		pResponse->strContent += "          charleft(document.postatalk,'talk_left','submit_talk');\n";
//		pResponse->strContent += "          window.location.hash = \"#\";\n";
//		pResponse->strContent += "          document.postatalk.talk.focus();\n";
//		pResponse->strContent += "          break; }\n";
//		pResponse->strContent += "      }\n";
//		pResponse->strContent += "    }\n";
//		pResponse->strContent += "  }\n";
//		pResponse->strContent += "  xmlhttp.open(\"GET\",rtForm.action + get_data,true);\n";
//		pResponse->strContent += "  xmlhttp.send();\n";
//		pResponse->strContent += "}\n\n";
		
		// load
		pResponse->strContent += "function load(tag,id,url,timeoutId) {\n";
		pResponse->strContent += "  var loadElement = document.getElementById( id );\n";
		pResponse->strContent += "  var gettimeout = false;\n";
		pResponse->strContent += "  if(typeof(timeoutId)!='undefined')\n";
		pResponse->strContent += "    gettimeout = true;\n";
		pResponse->strContent += "  if( gettimeout ) {\n";
		pResponse->strContent += "    timeoutElement = document.getElementById( timeoutId );\n";
		pResponse->strContent += "    funcTimeout = call_timeout( timeoutId ); }\n";
		pResponse->strContent += "  xmlhttp.onreadystatechange=function() {\n";
		pResponse->strContent += "    if (xmlhttp.readyState==4 && xmlhttp.status==200) {\n";
		pResponse->strContent += "      var e = document.createElement('div');\n";
		pResponse->strContent += "      e.innerHTML = xmlhttp.responseText;\n";
		pResponse->strContent += "      var elements = e.getElementsByTagName(tag);\n";
		pResponse->strContent += "      for (var i = 0; i < elements.length; i++) {\n";
		pResponse->strContent += "        if (elements[i].id == id) {\n";
		pResponse->strContent += "          if( gettimeout )\n";
		pResponse->strContent += "            clearTimeout( the_timeout );\n";
		pResponse->strContent += "          html = elements[i].innerHTML;\n";
		pResponse->strContent += "          loadElement.innerHTML = html;\n";
		pResponse->strContent += "          var start=html.indexOf('<s'+'cript');\n";
		pResponse->strContent += "          var end=html.indexOf('</s'+'cript>',start);\n";
		pResponse->strContent += "          while( start != -1 ) {\n";
		pResponse->strContent += "            eval(html.substring(start+36,end-6));\n";
		pResponse->strContent += "            start=html.indexOf('<s'+'cript',end);\n";
		pResponse->strContent += "            end=html.indexOf('</s'+'cript>',start); }\n";
//		pResponse->strContent += "  	      window.location.hash = \"#\";\n";
//		pResponse->strContent += "          document.postatalk.talk.focus();\n";
		pResponse->strContent += "          break; }\n";
		pResponse->strContent += "      }\n";
		pResponse->strContent += "    }\n";
		pResponse->strContent += "  }\n";
		pResponse->strContent += "  xmlhttp.open(\"GET\",url,true);\n";
		pResponse->strContent += "  xmlhttp.send();\n";
		pResponse->strContent += "  if( gettimeout ) {\n";
		pResponse->strContent += "    timeoutElement.disabled = true;\n";
		pResponse->strContent += "    the_timeout= setTimeout( funcTimeout, 15000 ); }\n";
		pResponse->strContent += "}\n\n";
		
		// get reply
		pResponse->strContent += "function get_reply(talkID,getID) {\n";
		pResponse->strContent += "  var divTalk = document.getElementById( talkID );\n";
		pResponse->strContent += "  var divGet = document.getElementById( talkID+'_reply'+getID );\n";
		pResponse->strContent += "  var exist = false;\n";
		pResponse->strContent += "  xmlhttp.onreadystatechange=function() {\n";
		pResponse->strContent += "    if (xmlhttp.readyState==4 && xmlhttp.status==200) {\n";
		pResponse->strContent += "      var e = document.createElement('div');\n";
		pResponse->strContent += "      e.innerHTML = xmlhttp.responseText;\n";
		pResponse->strContent += "      var elements = e.getElementsByTagName('div');\n";
		pResponse->strContent += "      for (var i = 0; i < elements.length; i++) {\n";
		pResponse->strContent += "        if (elements[i].id == 'talk'+getID) {\n";
		pResponse->strContent += "          divTalk.innerHTML = divTalk.innerHTML + '<div id=\"' + talkID + '_reply' + getID + '\" class=\"talk_reply\">' + elements[i].innerHTML + '</div>';\n";
//		pResponse->strContent += "  	      window.location.hash = \"#\";\n";
//		pResponse->strContent += "          document.postatalk.talk.focus();\n";
		pResponse->strContent += "          exist = true;\n";
		pResponse->strContent += "          break; }\n";
		pResponse->strContent += "      }\n";
		pResponse->strContent += "      if(!exist)\n";
		pResponse->strContent += "          divTalk.innerHTML = divTalk.innerHTML + '<div id=\"' + talkID + '_reply' + getID + '\" class=\"talk_reply\">" + UTIL_Xsprintf( gmapLANG_CFG["talk_not_exist"].c_str( ), "' + getID + '" ) + "</div>';\n";
		pResponse->strContent += "    }\n";
		pResponse->strContent += "  }\n";
		pResponse->strContent += "  if(!divGet) {\n";
		pResponse->strContent += "    xmlhttp.open(\"GET\",'" + RESPONSE_STR_TALK_HTML + "?id=' + getID,true);\n";
		pResponse->strContent += "    xmlhttp.send(); }\n";
		pResponse->strContent += "  else {\n";
		pResponse->strContent += "    if( divGet.style.display == \"none\" )\n";
		pResponse->strContent += "      divGet.style.display = \"\";\n";
		pResponse->strContent += "    else\n";
		pResponse->strContent += "      divGet.style.display = \"none\"; }\n";
		pResponse->strContent += "}\n\n";
		
		// get rt
		pResponse->strContent += "function get_rt(talkID,getID) {\n";
		pResponse->strContent += "  var divTalk = document.getElementById( talkID );\n";
		pResponse->strContent += "  if(!divTalk)\n";
		pResponse->strContent += "    divTalk = document.getElementById( talkID+'_rt'+getID );\n";
		pResponse->strContent += "  var divGet = document.getElementById( talkID+'_rt'+getID );\n";
		pResponse->strContent += "  var exist = false;\n";
		pResponse->strContent += "  xmlhttp.onreadystatechange=function() {\n";
		pResponse->strContent += "    if (xmlhttp.readyState==4 && xmlhttp.status==200) {\n";
		pResponse->strContent += "      var e = document.createElement('div');\n";
		pResponse->strContent += "      e.innerHTML = xmlhttp.responseText;\n";
		pResponse->strContent += "      var elements = e.getElementsByTagName('div');\n";
		pResponse->strContent += "      for (var i = 0; i < elements.length; i++) {\n";
		pResponse->strContent += "        if (elements[i].id == 'talk'+getID) {\n";
		pResponse->strContent += "          divTalk.innerHTML = divTalk.innerHTML + '<div id=\"' + talkID + '_rt' + getID + '\" class=\"talk_rt\">' + elements[i].innerHTML + '</div>';\n";
//		pResponse->strContent += "  	      window.location.hash = \"#\";\n";
//		pResponse->strContent += "          document.postatalk.talk.focus();\n";
		pResponse->strContent += "          exist = true;\n";
		pResponse->strContent += "          break; }\n";
		pResponse->strContent += "      }\n";
		pResponse->strContent += "      if(!exist)\n";
		pResponse->strContent += "          divTalk.innerHTML = divTalk.innerHTML + '<div id=\"' + talkID + '_rt' + getID + '\" class=\"talk_rt\">" + UTIL_Xsprintf( gmapLANG_CFG["talk_not_exist"].c_str( ), "' + getID + '" ) + "</div>';\n";
		pResponse->strContent += "    }\n";
		pResponse->strContent += "  }\n";
		pResponse->strContent += "  if(!divGet) {\n";
		pResponse->strContent += "    xmlhttp.open(\"GET\",'" + RESPONSE_STR_TALK_HTML + "?id=' + getID,true);\n";
		pResponse->strContent += "    xmlhttp.send(); }\n";
		pResponse->strContent += "  else {\n";
		pResponse->strContent += "    if( divGet.style.display == \"none\" )\n";
		pResponse->strContent += "      divGet.style.display = \"\";\n";
		pResponse->strContent += "    else\n";
		pResponse->strContent += "      divGet.style.display = \"none\"; }\n";
		pResponse->strContent += "}\n\n";
		
		// get torrent
		pResponse->strContent += "function get_torrent(talkID,torrentID) {\n";
		pResponse->strContent += "  var divTalk = document.getElementById( talkID );\n";
		pResponse->strContent += "  var divTorrent = document.getElementById( talkID+'_torrent'+torrentID );\n";
		pResponse->strContent += "  var exist = false;\n";
		pResponse->strContent += "  xmlhttp.onreadystatechange=function() {\n";
		pResponse->strContent += "    if (xmlhttp.readyState==4 && xmlhttp.status==200) {\n";
		pResponse->strContent += "      var e = document.createElement('div');\n";
		pResponse->strContent += "      e.innerHTML = xmlhttp.responseText;\n";
		pResponse->strContent += "      var elements = e.getElementsByTagName('div');\n";
		pResponse->strContent += "      for (var i = 0; i < elements.length; i++) {\n";
		pResponse->strContent += "        if (elements[i].id == 'torrent'+torrentID) {\n";
		pResponse->strContent += "          divTalk.innerHTML = divTalk.innerHTML + '<div id=\"' + talkID + '_torrent' + torrentID + '\" class=\"talk_torrent\">' + elements[i].innerHTML + '</div>';\n";
//		pResponse->strContent += "  	      window.location.hash = \"#\";\n";
//		pResponse->strContent += "          document.postatalk.talk.focus();\n";
		pResponse->strContent += "          exist = true;\n";
		pResponse->strContent += "          break; }\n";
		pResponse->strContent += "      }\n";
		pResponse->strContent += "      if(!exist)\n";
		pResponse->strContent += "          divTalk.innerHTML = divTalk.innerHTML + '<div id=\"' + talkID + '_torrent' + torrentID + '\" class=\"talk_torrent\">" + UTIL_Xsprintf( gmapLANG_CFG["torrent_not_exist"].c_str( ), "' + torrentID + '" ) + "</div>';\n";
		pResponse->strContent += "    }\n";
		pResponse->strContent += "  }\n";
		pResponse->strContent += "  if(!divTorrent) {\n";
		pResponse->strContent += "    xmlhttp.open(\"GET\",'" + RESPONSE_STR_TALK_HTML + "?tid=' + torrentID,true);\n";
//		pResponse->strContent += "    xmlhttp.open(\"GET\",'" + RESPONSE_STR_TALK_HTML + "?tid=' + torrentID + '&from=' + talkID,true);\n";
		pResponse->strContent += "    xmlhttp.send(); }\n";
		pResponse->strContent += "  else {\n";
		pResponse->strContent += "    if( divTorrent.style.display == \"none\" )\n";
		pResponse->strContent += "      divTorrent.style.display = \"\";\n";
		pResponse->strContent += "    else\n";
		pResponse->strContent += "      divTorrent.style.display = \"none\"; }\n";
		pResponse->strContent += "}\n\n";
		
		// return
//		pResponse->strContent += "function return(talkID,returnID) {\n";
//		pResponse->strContent += "  var trTalk = document.getElementById( talkID );\n";
//		pResponse->strContent += "  var spanReturn = document.getElementById( returnID );\n";
//		pResponse->strContent += "  trTalk.innerHTML = spanReturn.innerHTML;\n";
//		pResponse->strContent += "}\n\n";
		
		// set value
		pResponse->strContent += "function setValue(id,value) {\n";
		pResponse->strContent += "  document.getElementById( id ).value = value;\n";
		pResponse->strContent += "}\n\n";
		
		// clear all
		pResponse->strContent += "function clearAll() {\n";
		pResponse->strContent += "  setValue( 'talkarea', '' );\n";
		pResponse->strContent += "  setValue( 'inputReply', '' );\n";
		pResponse->strContent += "  setValue( 'inputReplyTo', '' );\n";
		pResponse->strContent += "  setValue( 'inputRT', '' );\n";
		pResponse->strContent += "  setValue( 'inputRTTo', '' );\n";
		pResponse->strContent += "  charleft(document.postatalk,'talk_left','submit_talk');\n";
		pResponse->strContent += "  document.postatalk.talk.focus();\n";
		pResponse->strContent += "}\n\n";
		
		// hide
		pResponse->strContent += "function hide(id) {\n";
		pResponse->strContent += "  var element = document.getElementById( id );\n";
		pResponse->strContent += "  element.style.visibility=\"hidden\";\n";
		pResponse->strContent += "}\n\n";
		
		// display
		pResponse->strContent += "function display(id) {\n";
		pResponse->strContent += "  var element = document.getElementById( id );\n";
		pResponse->strContent += "  element.style.visibility=\"visible\";\n";
		pResponse->strContent += "}\n\n";
		
		// friend
		pResponse->strContent += "function friend(id,friend_link,nofriend_link) {\n";
		pResponse->strContent += "  var friendLink = document.getElementById( 'friend'+id );\n";
		pResponse->strContent += "  xmlhttp.onreadystatechange=function() {\n";
		pResponse->strContent += "    if (xmlhttp.readyState==4) {\n";
		pResponse->strContent += "      if (xmlhttp.status==200) {\n";
		pResponse->strContent += "        if (friendLink.innerHTML == friend_link)\n";
		pResponse->strContent += "          friendLink.innerHTML = nofriend_link;\n";
		pResponse->strContent += "        else\n";
		pResponse->strContent += "          friendLink.innerHTML = friend_link;\n";
		pResponse->strContent += "      }\n";
		pResponse->strContent += "    }\n";
		pResponse->strContent += "  }\n";
		pResponse->strContent += "  if (friendLink.innerHTML == friend_link) {\n";
		pResponse->strContent += "    xmlhttp.open(\"GET\",\"" + RESPONSE_STR_LOGIN_HTML + "?friend=\" + id,true);\n";
		pResponse->strContent += "    xmlhttp.send(); }\n";
		pResponse->strContent += "  else {\n";
		pResponse->strContent += "    xmlhttp.open(\"GET\",\"" + RESPONSE_STR_LOGIN_HTML + "?nofriend=\" + id,true);\n";
		pResponse->strContent += "    xmlhttp.send(); }\n";
		pResponse->strContent += "}\n";
		
		// reply
//		pResponse->strContent += "function reply( commentid ) {\n";
//		pResponse->strContent += "var comment = document.getElementById( \"comment\" + commentid );\n";
//		pResponse->strContent += "var commenter = document.getElementById( \"commenter\" + commentid );\n";
//		pResponse->strContent += "var anchor = document.getElementById( \"commentarea\" );\n";
//		pResponse->strContent += "anchor.value = \"[quote=\" + commenter.innerHTML + \"]\" + comment.innerHTML + \"[/quote]\";\n";
//		pResponse->strContent += "window.location.hash = \"commentarea\";\n";
//		pResponse->strContent += "anchor.focus();\n";
//		pResponse->strContent += "}\n";
		pResponse->strContent += "//-->\n";
		pResponse->strContent += "</script>\n\n";
		
		string strUID( pRequest->mapParams["uid"] );
		string strTalkID( pRequest->mapParams["id"] );
		string strTorrentID( pRequest->mapParams["tid"] );
		const string cstrTag( pRequest->mapParams["tag"] );
		const string cstrTalk( pRequest->mapParams["talk"] );
		const string cstrShow( pRequest->mapParams["show"] );
		const string cstrPage( pRequest->mapParams["page"] );
		const string cstrPerPage( pRequest->mapParams["per_page"] );
//		const string cstrFromID( pRequest->mapParams["from"] );
//		const string cstrType( pRequest->mapParams["type"] );
		const string cstrAutoload( pRequest->mapParams["autoload"] );
		
		if( strUID.find( " " ) != string :: npos )
			strUID.erase( );
			
		if( strTalkID.find( " " ) != string :: npos )
			strTalkID.erase( );
			
		if( strTorrentID.find( " " ) != string :: npos )
			strTorrentID.erase( );
		
//		string strReply( pRequest->mapParams["reply"] );
//		string strRT( pRequest->mapParams["rt"] );
//		
//		string strReplyTo = string( );
//		string strReplyCom = string( );
//		if( strReply.find( " " ) != string :: npos )
//			strReply.erase( );
//		if( strRT.find( " " ) != string :: npos )
//			strRT.erase( );

		bool bNoComment = false;
		
		if( ( pRequest->user.ucAccess & m_ucAccessComments ) )
		{
			pResponse->strContent += "<div class=\"talk_table_post\">\n";
			
			pResponse->strContent += "<form id=\"postForm\" method=\"post\" action=\"" + RESPONSE_STR_TALK_HTML + "\" name=\"postatalk\" onSubmit=\"return validate( this )\" enctype=\"multipart/form-data\">\n";
			pResponse->strContent += "<table class=\"talk_table_post\">\n";
			
//			if( !strReply.empty( ) )
				pResponse->strContent += "<input id=\"inputReply\" type=\"hidden\" name=\"reply\" value=\"\">\n";
				pResponse->strContent += "<input id=\"inputReplyTo\" type=\"hidden\" name=\"replyto\" value=\"\">\n";
//			if( !strRT.empty( ) )
				pResponse->strContent += "<input id=\"inputRT\" type=\"hidden\" name=\"rt\" value=\"\">\n";
				pResponse->strContent += "<input id=\"inputRTTo\" type=\"hidden\" name=\"rtto\" value=\"\">\n";
			if( !strUID.empty( ) )
				pResponse->strContent += "<input type=\"hidden\" name=\"uid\" value=\"" + strUID + "\">\n";
			if( !cstrShow.empty( ) )
				pResponse->strContent += "<input type=\"hidden\" name=\"show\" value=\"" + cstrShow + "\">\n";
			if( !cstrTag.empty( ) )
				pResponse->strContent += "<input type=\"hidden\" name=\"tag\" value=\"" + cstrTag + "\">\n";
//			if( !cstrPage.empty( ) )
//				pResponse->strContent += "<input type=\"hidden\" name=\"page\" value=\"" + cstrPage + "\">";
			if( !cstrPerPage.empty( ) )
				pResponse->strContent += "<input type=\"hidden\" name=\"per_page\" value=\"" + cstrPerPage + "\">";
			
			pResponse->strContent += "<tr class=\"talk_table_post\">\n";
			pResponse->strContent += "<td class=\"talk_table_header\">\n";
			pResponse->strContent += "<span class=\"talk_table_header\">" + gmapLANG_CFG["talk"] + ":</span>";
			pResponse->strContent += "</td>\n";
			pResponse->strContent += "<td class=\"talk_table_left\">\n";
			pResponse->strContent += "<input name=\"clear_talk_button\" id=\"clear_talk\" alt=\"[" + gmapLANG_CFG["talk_clear"] + "]\" type=button value=\"" + gmapLANG_CFG["talk_clear"] + "\" onClick=\"javascript: clearAll();\">\n";
			pResponse->strContent += "<span id=\"talk_left\" class=\"talk_table_left\">" + UTIL_Xsprintf( gmapLANG_CFG["talk_left"].c_str( ), CAtomInt( m_uiTalkLength ).toString( ).c_str( ) ) + "</span>\n";
			pResponse->strContent += "</td>\n</tr>\n";
			
			pResponse->strContent += "<tr class=\"talk_table_post\">\n";
			pResponse->strContent += "<td class=\"talk_table_post\" colspan=2>\n";
			pResponse->strContent += "<textarea class=\"talk\" id=\"talkarea\" name=\"talk\" rows=8 cols=64 onInput=\"javascript: charleft(document.postatalk,'talk_left','submit_talk');\" onPropertyChange=\"javascript: charleft(document.postatalk,'talk_left','submit_talk');\" onKeyDown=\"javascript: keypost(event,'submit_talk');\">";
			
//			if( !strReply.empty( ) || !strRT.empty( ) )
//			{
//				CMySQLQuery *pQueryTalk = 0;
//				
//				if( !strReply.empty( ) )
//					pQueryTalk = new CMySQLQuery( "SELECT busername,btalkstore FROM talk WHERE bid=" + strReply );
//				else
//					pQueryTalk = new CMySQLQuery( "SELECT busername,btalkstore FROM talk WHERE bid=" + strRT );
//				
//				vector<string> vecQueryTalk;
//		
//				vecQueryTalk.reserve(2);

//				vecQueryTalk = pQueryTalk->nextRow( );
//				
//				delete pQueryTalk;
//				
//				if( vecQueryTalk.size( ) == 2 && !vecQueryTalk[0].empty( ) )
//				{
//					strReplyTo = vecQueryTalk[0];
//					strReplyCom = vecQueryTalk[1];
//				}
//				
//				if( !strReplyCom.empty( ) )
//				{
//					if( !strReply.empty( ) )
//						pResponse->strContent += "@" + UTIL_RemoveHTML( strReplyTo ) + " ";
//					else if( !strRT.empty( ) )
//						pResponse->strContent += UTIL_Xsprintf( gmapLANG_CFG["talk_rt_content"].c_str( ), UTIL_RemoveHTML( strReplyTo ).c_str( ), UTIL_RemoveHTML3( strReplyCom ).c_str( ) );
//				}
//			}

			pResponse->strContent += "</textarea>\n";
			pResponse->strContent += "</td>\n</tr>\n";
			
			pResponse->strContent += "<tr class=\"talk_table_post\">\n";
			pResponse->strContent += "<td class=\"talk_table_post\" colspan=2>\n";
			pResponse->strContent += "<div class=\"talk_table_post_button\">\n";
			pResponse->strContent += "<input name=\"submit_talk_button\" id=\"submit_talk\" alt=\"" + gmapLANG_CFG["talk"] + "\" type=button value=\"" + gmapLANG_CFG["talk"] + "\" onClick=\"javascript: post('postForm','talkarea','submit_talk');\">\n";
//			pResponse->strContent += Button_Submit( "submit_talk", string( gmapLANG_CFG["Submit"] ) );
			pResponse->strContent += "</div>\n</td>\n</tr>\n";

			pResponse->strContent += "</table>\n";
			pResponse->strContent += "</form>\n";
			pResponse->strContent += "</div>\n";
			
//			if( !strReply.empty( ) || !strRT.empty( ) )
//			{
//				// Output common HTML tail
//				HTML_Common_End( pRequest, pResponse, btv, NOT_INDEX, string( CSS_TALK ) );
//				
//				return;
//			}
		}
		else
			pResponse->strContent += "<p class=\"denied\">" + gmapLANG_CFG["talk_post_disallowed"] + "</p>\n";
		
		vector< pair< string, string > > vecParams;
		vecParams.reserve(64);
		string strJoined = string( );

		vecParams.push_back( pair<string, string>( string( "uid" ), strUID ) );
		vecParams.push_back( pair<string, string>( string( "show" ), cstrShow ) );
		vecParams.push_back( pair<string, string>( string( "page" ), cstrPage ) );
		vecParams.push_back( pair<string, string>( string( "per_page" ), cstrPerPage ) );
		
		strJoined = UTIL_RemoveHTML( UTIL_HTMLJoin( vecParams, string( "?" ), string( "&" ), string( "=" ) ) );
		
		pResponse->strContent += "<div id=\"divTalk\">";

		unsigned long ulCount = 0;
		unsigned long ulStart = 0;
		unsigned long ulLimit = 0;
		unsigned int uiOverridePerPage = 0;
		
		if ( cstrPerPage.empty( ) )
		{
			if( pRequest->mapCookies["per_page"].empty( ) )
				uiOverridePerPage = m_uiPerPage;
			else
			{
				uiOverridePerPage = (unsigned int)atoi( pRequest->mapCookies["per_page"].c_str( ) );

				if( uiOverridePerPage < 1 || uiOverridePerPage > m_uiPerPageMax )
					uiOverridePerPage = m_uiPerPage;
			}
		}
		else
		{
			uiOverridePerPage = (unsigned int)atoi( cstrPerPage.c_str( ) );

			if( uiOverridePerPage > m_uiPerPageMax )
				uiOverridePerPage = m_uiPerPage;
		}
		
//		if( !cstrPage.empty( ) )
//			ulStart = (unsigned long)atoi( cstrPage.c_str( ) ) * uiOverridePerPage;
			
		if( !cstrPage.empty( ) )
			ulLimit = (unsigned long)atoi( cstrPage.c_str( ) ) * uiOverridePerPage;
		
		ulLimit += uiOverridePerPage;
		
		CMySQLQuery *pQueryUser = new CMySQLQuery( "SELECT btalk,btalkref,btalktorrent FROM users WHERE buid=" + pRequest->user.strUID );
				
		vector<string> vecQueryUser;
	
		vecQueryUser.reserve(3);
		
		vecQueryUser = pQueryUser->nextRow( );
		
		delete pQueryUser;
		
		if( vecQueryUser.size( ) == 3 )
		{
			pResponse->strContent += "<p class=\"subfilter\">";
		
			pResponse->strContent += "<a";
			if( cstrShow.empty( ) && cstrTag.empty( ) && strUID.empty( ) && strTalkID.empty( ) && strTorrentID.empty( ) )
				pResponse->strContent += " class=\"blue\"";
			pResponse->strContent += " href=\"" + RESPONSE_STR_TALK_HTML + "\">" + gmapLANG_CFG["talk_show_home"] + "</a>";
	//				pResponse->strContent += "<a href=\"javascript: ;\" onClick=\"javascrip: load('div','divTalk','" + RESPONSE_STR_TALK_HTML + "');\">" + gmapLANG_CFG["talk_show_home"] + "</a>";
			if( vecQueryUser[0] != "0" )
				pResponse->strContent += "<span class=\"hot\">(" + vecQueryUser[0] + ")</span>";
			pResponse->strContent += "<span class=\"pipe\"> | </span>";
		
			pResponse->strContent += "<a";
			if( cstrShow == "mentions" )
				pResponse->strContent += " class=\"blue\"";
			pResponse->strContent += " href=\"" + RESPONSE_STR_TALK_HTML + "?show=mentions\">" + gmapLANG_CFG["talk_show_mentions"] + "</a>";
	//				pResponse->strContent += "<a href=\"javascript: ;\" onClick=\"javascrip: load('div','divTalk','" + RESPONSE_STR_TALK_HTML + "?show=mentions');\">" + gmapLANG_CFG["talk_show_mentions"] + "</a>";
			if( vecQueryUser[1] != "0" )
				pResponse->strContent += "<span class=\"hot\">(" + vecQueryUser[1] + ")</span>";
			pResponse->strContent += "<span class=\"pipe\"> | </span>";
			
			pResponse->strContent += "<a";
			if( cstrShow == "torrents" )
				pResponse->strContent += " class=\"blue\"";
			pResponse->strContent += " href=\"" + RESPONSE_STR_TALK_HTML + "?show=torrents\">" + gmapLANG_CFG["talk_show_torrents"] + "</a>";
	//				pResponse->strContent += "<a href=\"javascript: ;\" onClick=\"javascrip: load('div','divTalk','" + RESPONSE_STR_TALK_HTML + "?show=mentions');\">" + gmapLANG_CFG["talk_show_mentions"] + "</a>";
			if( vecQueryUser[2] != "0" )
				pResponse->strContent += "<span class=\"hot\">(" + vecQueryUser[2] + ")</span>";
			pResponse->strContent += "<span class=\"pipe\"> | </span>";
			
			pResponse->strContent += "<a";
			if( cstrShow == "all" )
				pResponse->strContent += " class=\"blue\"";
			pResponse->strContent += " href=\"" + RESPONSE_STR_TALK_HTML + "?show=all\">" + gmapLANG_CFG["talk_show_all"] + "</a>";
	//				pResponse->strContent += "<a href=\"javascript: ;\" onClick=\"javascrip: load('div','divTalk','" + RESPONSE_STR_TALK_HTML + "?show=all');\">" + gmapLANG_CFG["talk_show_all"] + "</a>";

			pResponse->strContent += "</p>\n";
		}
		
		//
		// delete talk
		//

		if( pRequest->user.ucAccess & m_ucAccessDelOwn )
		{
			string cstrDel( pRequest->mapParams["del"] );

			if( cstrDel.find( " " ) != string :: npos )
				cstrDel.erase( );

			if( !cstrDel.empty( ) )
			{
				CMySQLQuery *pQueryTalk = new CMySQLQuery( "SELECT buid FROM talk WHERE bid=" + cstrDel );
			
				vector<string> vecQueryTalk;
			
				vecQueryTalk.reserve(1);

				vecQueryTalk = pQueryTalk->nextRow( );
				
				delete pQueryTalk;

				if( vecQueryTalk.size( ) == 1 )
				{
					if( vecQueryTalk[0] == pRequest->user.strUID )
					{
//						CMySQLQuery *pQueryRef = new CMySQLQuery( "SELECT buid FROM talkref WHERE brefid=" + cstrDel );
//			
//						vector<string> vecQueryRef;
//	
//						vecQueryRef.reserve(1);

//						vecQueryRef = pQueryRef->nextRow( );
//				
//						while( vecQueryRef.size( ) == 1 )
//						{
//							if( !vecQueryRef[0].empty( ) )
//								CMySQLQuery mq04( "UPDATE users SET btalkref=btalkref-1 WHERE buid=" + vecQueryRef[0] + " AND btalkref>0" );
//						
//							vecQueryRef = pQueryRef->nextRow( );
//						}
//		
//						delete pQueryRef;
						
						CMySQLQuery mq01( "DELETE FROM talkref WHERE brefid=" + cstrDel );

//						CMySQLQuery *pQueryHome = new CMySQLQuery( "SELECT buid FROM talkhome WHERE btalkid=" + cstrDel );
//			
//						vector<string> vecQueryHome;
//	
//						vecQueryHome.reserve(1);

//						vecQueryHome = pQueryHome->nextRow( );
//				
//						while( vecQueryHome.size( ) == 1 )
//						{
//							if( !vecQueryHome[0].empty( ) && vecQueryHome[0] != pRequest->user.strUID )
//								CMySQLQuery mq04( "UPDATE users SET btalk=btalk-1 WHERE buid=" + vecQueryHome[0] + " AND btalk>0" );
//						
//							vecQueryHome = pQueryHome->nextRow( );
//						}
//		
//						delete pQueryHome;
						
						CMySQLQuery mq02( "DELETE FROM talkhome WHERE btalkid=" + cstrDel );
						
						CMySQLQuery mq03( "DELETE FROM talktag WHERE bid=" + cstrDel );
						
						CMySQLQuery mq04( "DELETE FROM talk WHERE bid=" + cstrDel );
						
						// Deleted talk
						pResponse->strContent += "<p class=\"deleted\">" + UTIL_Xsprintf( gmapLANG_CFG["talk_deleted"].c_str( ), cstrDel.c_str( ) ) + "</p>\n";

						// Output common HTML tail
//						HTML_Common_End( pRequest, pResponse, btv, NOT_INDEX, string( CSS_TALK ) );

//						return;
					}
				}
				else
				{
					// Unable to delete talk
					pResponse->strContent += "<p class=\"delete\">" + UTIL_Xsprintf( gmapLANG_CFG["talk_unable_delete"].c_str( ), cstrDel.c_str( ) ) + "</p>\n";

					// Output common HTML tail
	//				HTML_Common_End( pRequest, pResponse, btv, NOT_INDEX, string( CSS_TALK ) );

	//				return;
				}
			}
		}
		
		pResponse->strContent += "<div id=\"divTalkTable\" class=\"talk_table_posted\">";
		
		unsigned long ulAdded = 0;
		unsigned long ulSkipped = 0;
		
		CMySQLQuery *pQueryTalk = 0;
		
		if( !strTalkID.empty( ) )
		{
			CMySQLQuery *pQueryOne = new CMySQLQuery( "SELECT bid,busername,buid,bposted,btalk,btalkstore,breply,breplyto,breplytoid,brt,brtto,brttoid FROM talk WHERE bid=" + strTalkID );
			
			vector<string> vecQueryOne;

			vecQueryOne.reserve(12);

			vecQueryOne = pQueryOne->nextRow( );
			
			delete pQueryOne;
			
			if( vecQueryOne.size( ) == 12 )
			{
				pResponse->strContent += "<table class=\"talk_table_posted\" summary=\"talk\">\n";
				pResponse->strContent += GenerateTalk( vecQueryOne, pRequest->user.ucAccess, pRequest->user.strUID, strJoined );
				
				pResponse->strContent += "</table>\n";
				
				ulCount++;
			}
		}
		else if( !strTorrentID.empty( ) )
		{
			CMySQLQuery *pQueryTorrent = new CMySQLQuery( "SELECT bname,badded,btitle,buploader,buploaderid,bseeders,bleechers,bcompleted FROM allowed WHERE bid=" + strTorrentID );

			vector<string> vecQueryTorrent;

			vecQueryTorrent.reserve(8);

			vecQueryTorrent = pQueryTorrent->nextRow( );
			
			delete pQueryTorrent;
			
			if( vecQueryTorrent.size( ) == 8 )
			{
//				const string cstrFromID( pRequest->mapParams["from"] );
				
				string strUserLink = getUserLink( vecQueryTorrent[4], vecQueryTorrent[3] );
				string strName = string( );
				string strEngName = string( );
				string strChiName = string( );
				
				if( !vecQueryTorrent[0].empty( ) )
					strName = vecQueryTorrent[0];
				if( !vecQueryTorrent[2].empty( ) )
					strName = vecQueryTorrent[2];
					
				pResponse->strContent += "<table class=\"talk_table_posted\" summary=\"talk\">\n";
				pResponse->strContent += "<tr class=\"talk_body\">";
				pResponse->strContent += "<td class=\"talk_body\">\n<div id=\"torrent" + strTorrentID + "\">";
				UTIL_StripName( strName.c_str( ), strEngName, strChiName );
				pResponse->strContent += "<a class=\"talk_torrent\" target=\"_blank\" title=\"" + UTIL_RemoveHTML( strName ) + "\" href=\"" + RESPONSE_STR_STATS_HTML + "?id=" + strTorrentID + "\">" + UTIL_RemoveHTML( strEngName );
				if( !strChiName.empty( ) )
					pResponse->strContent += "<br>" + UTIL_RemoveHTML( strChiName );
				pResponse->strContent += "</a>";
//					pResponse->strContent += "<br><a class=\"talk_time\" href=\"javascript: ;\"";
//					if( !cstrFromID.empty( ) )
//						pResponse->strContent += " onClick=\"return('talk" + cstrFromID + "','return" + cstrFromID + "');\"";
//					pResponse->strContent += ">" + vecQueryTorrent[1] + "</a>";
				pResponse->strContent += "<br><span class=\"talk_torrent\">" + UTIL_Xsprintf( gmapLANG_CFG["talk_torrent_status"].c_str( ), vecQueryTorrent[5].c_str( ), vecQueryTorrent[6].c_str( ), vecQueryTorrent[7].c_str( ) ) + "</span>";
				pResponse->strContent += "<br>" + UTIL_Xsprintf( gmapLANG_CFG["talk_torrent_added"].c_str( ), strUserLink.c_str( ), vecQueryTorrent[1].c_str( ) );
				pResponse->strContent += " | " + gmapLANG_CFG["talk_tag_torrent"] + "<a class=\"talk_tag\" href=\"" + RESPONSE_STR_TALK_HTML + "?tag=" + UTIL_StringToEscaped( gmapLANG_CFG["torrent"] + strTorrentID ) + "\">" + UTIL_RemoveHTML( "#" + gmapLANG_CFG["torrent"] + strTorrentID + "#" ) + "</a>";
				pResponse->strContent += "</div>\n</td>\n";
//					pResponse->strContent += "<td class=\"talk_function\"></td>\n";
				pResponse->strContent += "</tr>\n";
				pResponse->strContent += "</table>\n";
				
				ulCount++;
			}
		}
		else if( cstrShow == "mentions" )
		{
			pQueryTalk = new CMySQLQuery( "SELECT brefid FROM talkref WHERE buid=" + pRequest->user.strUID + " ORDER BY bposted DESC LIMIT " + CAtomLong( ulLimit ).toString( ) );
			
			if( cstrAutoload.empty( ) )
				CMySQLQuery mq01( "UPDATE users SET btalkref=0 WHERE buid=" + pRequest->user.strUID );
				
			vector<string> vecQueryTalk;
	
			vecQueryTalk.reserve(1);

			vecQueryTalk = pQueryTalk->nextRow( );
		
			if( vecQueryTalk.size( ) == 1 && !vecQueryTalk[0].empty( ) )
			{
				pResponse->strContent += "<table class=\"talk_table_posted\" summary=\"talk\">\n";
			
				while( vecQueryTalk.size( ) == 1 )
				{
					CMySQLQuery *pQueryRef = new CMySQLQuery( "SELECT bid,busername,buid,bposted,btalk,btalkstore,breply,breplyto,breplytoid,brt,brtto,brttoid FROM talk WHERE bid=" + vecQueryTalk[0] );
					
					vector<string> vecQueryRef;
	
					vecQueryRef.reserve(12);

					vecQueryRef = pQueryRef->nextRow( );
					
					delete pQueryRef;
					
					if( vecQueryRef.size( ) == 12 )
					{
						pResponse->strContent += GenerateTalk( vecQueryRef, pRequest->user.ucAccess, pRequest->user.strUID, strJoined );

						ulAdded++;
					}
					ulCount++;
					
					vecQueryTalk = pQueryTalk->nextRow( );
				}
				pResponse->strContent += "</table>\n";
			}
		}
		else if( !cstrTag.empty( ) )
		{
			pResponse->strContent += "<div class=\"talk_tag\">" + UTIL_Xsprintf( gmapLANG_CFG["talk_tag_about"].c_str( ), string( "<span class=\"talk_tag\">" + UTIL_RemoveHTML( cstrTag ) + "</span>" ).c_str( ) ) + "</div>\n";
			
			pQueryTalk = new CMySQLQuery( "SELECT bid FROM talktag WHERE btag='" + UTIL_StringToMySQL( cstrTag ) + "' ORDER BY bposted DESC LIMIT " + CAtomLong( ulLimit ).toString( ) );
				
			vector<string> vecQueryTalk;
	
			vecQueryTalk.reserve(1);

			vecQueryTalk = pQueryTalk->nextRow( );
		
			if( vecQueryTalk.size( ) == 1 && !vecQueryTalk[0].empty( ) )
			{
				pResponse->strContent += "<table class=\"talk_table_posted\" summary=\"talk\">\n";
			
				while( vecQueryTalk.size( ) == 1 )
				{
					CMySQLQuery *pQueryRef = new CMySQLQuery( "SELECT bid,busername,buid,bposted,btalk,btalkstore,breply,breplyto,breplytoid,brt,brtto,brttoid FROM talk WHERE bid=" + vecQueryTalk[0] );
					
					vector<string> vecQueryRef;
	
					vecQueryRef.reserve(12);

					vecQueryRef = pQueryRef->nextRow( );
					
					delete pQueryRef;
					
					if( vecQueryRef.size( ) == 12 )
					{
						pResponse->strContent += GenerateTalk( vecQueryRef, pRequest->user.ucAccess, pRequest->user.strUID, strJoined );

						ulAdded++;
					}
					ulCount++;
					
					vecQueryTalk = pQueryTalk->nextRow( );
				}
				pResponse->strContent += "</table>\n";
			}
		}
		else if( cstrShow == "torrents" )
		{
			pQueryTalk = new CMySQLQuery( "SELECT btid FROM talktorrent WHERE buid=" + pRequest->user.strUID + " ORDER BY bposted DESC LIMIT " + CAtomLong( ulLimit ).toString( ) );
			
			if( cstrAutoload.empty( ) )
				CMySQLQuery mq01( "UPDATE users SET btalktorrent=0 WHERE buid=" + pRequest->user.strUID );
				
			vector<string> vecQueryTalk;
	
			vecQueryTalk.reserve(1);

			vecQueryTalk = pQueryTalk->nextRow( );
		
			if( vecQueryTalk.size( ) == 1 && !vecQueryTalk[0].empty( ) )
			{
				pResponse->strContent += "<table class=\"talk_table_posted\" summary=\"talk\">\n";
			
				while( vecQueryTalk.size( ) == 1 )
				{
					CMySQLQuery *pQueryTorrent = new CMySQLQuery( "SELECT bname,badded,btitle,buploader,buploaderid,bseeders,bleechers,bcompleted FROM allowed WHERE bid=" + vecQueryTalk[0] );

					vector<string> vecQueryTorrent;

					vecQueryTorrent.reserve(8);

					vecQueryTorrent = pQueryTorrent->nextRow( );
			
					delete pQueryTorrent;
			
					if( vecQueryTorrent.size( ) == 8 )
					{
						string strUserLink = getUserLink( vecQueryTorrent[4], vecQueryTorrent[3] );
						string strName = string( );
						string strEngName = string( );
						string strChiName = string( );
				
						if( !vecQueryTorrent[0].empty( ) )
							strName = vecQueryTorrent[0];
						if( !vecQueryTorrent[2].empty( ) )
							strName = vecQueryTorrent[2];
					
						pResponse->strContent += "<tr class=\"talk_body\">";
						pResponse->strContent += "<td class=\"talk_body\">\n<div id=\"torrent" + vecQueryTalk[0] + "\">";
						UTIL_StripName( strName.c_str( ), strEngName, strChiName );
						pResponse->strContent += "<a class=\"talk_torrent\" target=\"_blank\" title=\"" + UTIL_RemoveHTML( strName ) + "\" href=\"" + RESPONSE_STR_STATS_HTML + "?id=" + vecQueryTalk[0] + "\">" + UTIL_RemoveHTML( strEngName );
						if( !strChiName.empty( ) )
							pResponse->strContent += "<br>" + UTIL_RemoveHTML( strChiName );
						pResponse->strContent += "</a>";
						pResponse->strContent += "<br><span class=\"talk_torrent\">" + UTIL_Xsprintf( gmapLANG_CFG["talk_torrent_status"].c_str( ), vecQueryTorrent[5].c_str( ), vecQueryTorrent[6].c_str( ), vecQueryTorrent[7].c_str( ) ) + "</span>";
						
						pResponse->strContent += "<br>" + UTIL_Xsprintf( gmapLANG_CFG["talk_torrent_added"].c_str( ), strUserLink.c_str( ), vecQueryTorrent[1].c_str( ) );
						pResponse->strContent += "</div>\n</td>\n";
	//					pResponse->strContent += "<td class=\"talk_function\"></td>\n";
						pResponse->strContent += "</tr>\n";
					}
					ulCount++;
					
					vecQueryTalk = pQueryTalk->nextRow( );
				}
				pResponse->strContent += "</table>\n";
			}
		}
		else if( cstrShow.empty( ) && strUID.empty( ) )
		{
			pQueryTalk = new CMySQLQuery( "SELECT btalkid FROM talkhome WHERE buid=" + pRequest->user.strUID + " ORDER BY bposted DESC LIMIT " + CAtomLong( ulLimit ).toString( ) );
		
			if( cstrAutoload.empty( ) )
				CMySQLQuery mq01( "UPDATE users SET btalk=0 WHERE buid=" + pRequest->user.strUID );

			vector<string> vecQueryTalk;

			vecQueryTalk.reserve(1);

			vecQueryTalk = pQueryTalk->nextRow( );
	
			if( vecQueryTalk.size( ) == 1 && !vecQueryTalk[0].empty( ) )
			{
				pResponse->strContent += "<table class=\"talk_table_posted\" summary=\"talk\">\n";
		
				while( vecQueryTalk.size( ) == 1 )
				{
					CMySQLQuery *pQueryHome = new CMySQLQuery( "SELECT bid,busername,buid,bposted,btalk,btalkstore,breply,breplyto,breplytoid,brt,brtto,brttoid FROM talk WHERE bid=" + vecQueryTalk[0] );
				
					vector<string> vecQueryHome;

					vecQueryHome.reserve(12);

					vecQueryHome = pQueryHome->nextRow( );
				
					delete pQueryHome;
				
					if( vecQueryHome.size( ) == 12 )
					{
						pResponse->strContent += GenerateTalk( vecQueryHome, pRequest->user.ucAccess, pRequest->user.strUID, strJoined );

						ulAdded++;
					}
					ulCount++;
					
					vecQueryTalk = pQueryTalk->nextRow( );
				}
				pResponse->strContent += "</table>\n";
			}
		}
		else
		{
			if( !strUID.empty( ) )
				pQueryTalk = new CMySQLQuery( "SELECT bid,busername,buid,bposted,btalk,btalkstore,breply,breplyto,breplytoid,brt,brtto,brttoid FROM talk WHERE buid=" + strUID + " ORDER BY bposted DESC LIMIT " + CAtomLong( ulLimit ).toString( ) );
			else
				pQueryTalk = new CMySQLQuery( "SELECT bid,busername,buid,bposted,btalk,btalkstore,breply,breplyto,breplytoid,brt,brtto,brttoid FROM talk ORDER BY bposted DESC LIMIT " + CAtomLong( ulLimit ).toString( ) );
			
			vector<string> vecQueryTalk;
	
			vecQueryTalk.reserve(12);

			vecQueryTalk = pQueryTalk->nextRow( );
			
			bool bTalker = true;
			
			if( !strUID.empty( ) )

			{
				CMySQLQuery *pQueryUser = new CMySQLQuery( "SELECT busername FROM users WHERE buid=" + strUID );

				vector<string> vecQueryUser;
	
				vecQueryUser.reserve(1);

				vecQueryUser = pQueryUser->nextRow( );
	
				delete pQueryUser;
	
				if( vecQueryUser.size( ) == 1 )
				{
					pResponse->strContent += "<div class=\"talk_user\">\n<p>" + getUserLink( strUID, string( ) );
				
					if( pRequest->user.ucAccess & m_ucAccessComments )
					{
						pResponse->strContent += " <span>[<a class=\"black\" title=\"" + gmapLANG_CFG["talk_to"] + "\" href=\"" + RESPONSE_STR_TALK_HTML + "?talk=" + UTIL_StringToEscaped( "@" + vecQueryUser[0] + " " ) + "\">" + gmapLANG_CFG["talk_to"] + "</a>]</span>";
					}
				
					if( pRequest->user.ucAccess & m_ucAccessMessages )
					{
						pResponse->strContent += " <span>[<a class=\"black\" title=\"" + gmapLANG_CFG["messages_send_message"] + "\" href=\"" + RESPONSE_STR_MESSAGES_HTML + "?sendto=" + strUID + "\">" + gmapLANG_CFG["messages_send_message"] + "</a>]</span>";
					}
			
					if( pRequest->user.strUID != strUID )
					{
						pResponse->strContent += " <span>[<a class=\"friend\" id=\"friend" + strUID + "\" class=\"red\" href=\"javascript: ;\" onclick=\"javascript: friend('" + strUID + "','" + gmapLANG_CFG["friend_add"] + "','" + gmapLANG_CFG["friend_remove"] + "');\">";
			
						CMySQLQuery *pQueryFriend = new CMySQLQuery( "SELECT buid,bfriendid FROM friends WHERE buid=" + pRequest->user.strUID + " AND bfriendid=" + strUID );

						vector<string> vecQueryFriend;

						vecQueryFriend.reserve(2);

						vecQueryFriend = pQueryFriend->nextRow( );
			
						delete pQueryFriend;
			
						if( vecQueryFriend.size( ) == 2 && !vecQueryFriend[0].empty( ) )
							pResponse->strContent += gmapLANG_CFG["friend_remove"];
						else
							pResponse->strContent += gmapLANG_CFG["friend_add"];

						pResponse->strContent += "</a>]</span>";
					}
					
					pResponse->strContent += "</p></div>\n";
					
					bTalker = false;
				}
			}
			
//			ulCount = pQueryTalk->numRows( );
			
			if( vecQueryTalk.size( ) == 12 )
			{
				pResponse->strContent += "<table class=\"talk_table_posted\" summary=\"talk\">\n";
			
				while( vecQueryTalk.size( ) == 12 )
				{
					pResponse->strContent += GenerateTalk( vecQueryTalk, pRequest->user.ucAccess, pRequest->user.strUID, strJoined, bTalker );

					ulAdded++;

					ulCount++;
					
					vecQueryTalk = pQueryTalk->nextRow( );
				}
				pResponse->strContent += "</table>\n";
			}
		}
		
		if( ulCount == 0 )
			pResponse->strContent += "<p class=\"talk_table_posted\">" + gmapLANG_CFG["talk_no_talk"] + "</p>\n";
			
		delete pQueryTalk;
		
		vecParams.clear( );
		
		vecParams.push_back( pair<string, string>( string( "uid" ), strUID ) );
		vecParams.push_back( pair<string, string>( string( "show" ), cstrShow ) );
		vecParams.push_back( pair<string, string>( string( "per_page" ), cstrPerPage ) );
		
		strJoined = UTIL_RemoveHTML( UTIL_HTMLJoin( vecParams, string( "&" ), string( "&" ), string( "=" ) ) );
		
		if( ulCount == ulLimit )
			pResponse->strContent += "<p><a id=\"talk_more\" class=\"talk_more\" href=\"javascript: ;\" onClick=\"javascript: load('div','divTalkTable','" + RESPONSE_STR_TALK_HTML + "?page=" + CAtomInt( ulLimit / uiOverridePerPage ).toString( ) + strJoined + "','talk_more');\">" + gmapLANG_CFG["talk_more"] + "</a></p>";
		
		// page numbers
		
//		pResponse->strContent += UTIL_PageBarAJAX( ulCount, cstrPage, uiOverridePerPage, RESPONSE_STR_TALK_HTML, strJoined, "div", "divTalk", false );
		
		vecParams.push_back( pair<string, string>( string( "autoload" ), "1" ) );
		
		strJoined = UTIL_HTMLJoin( vecParams, string( "?" ), string( "&" ), string( "=" ) );
		
		pResponse->strContent += "<script type=\"text/javascript\">\n";
		pResponse->strContent += "<!--\n";
		pResponse->strContent += "setInterval( autoLoad, 300000 );\n";
		pResponse->strContent += "function autoLoad() {\n";
		pResponse->strContent += "  load('div','divTalk','" + RESPONSE_STR_TALK_HTML + strJoined + "');\n";
		pResponse->strContent += "}\n";
		pResponse->strContent += "//-->\n";
		pResponse->strContent += "</script>\n\n";
		
		pResponse->strContent += "</div>\n";
		
		pResponse->strContent += "<script type=\"text/javascript\">\n";
		pResponse->strContent += "<!--\n";
		pResponse->strContent += "document.postatalk.talk.focus();\n";
		if( !cstrTag.empty( ) )
		{
			pResponse->strContent += "var tagData = '" + UTIL_RemoveHTML3( "#" + cstrTag + "#" ) + "';\n";
			pResponse->strContent += "document.postatalk.talk.value = tagData.stripX( );\n";
		}
		pResponse->strContent += "charleft(document.postatalk,'talk_left','submit_talk');\n";
		pResponse->strContent += "//-->\n";
		pResponse->strContent += "</script>\n\n";

		pResponse->strContent += "</div>\n";
		
		if( !cstrTalk.empty( ) )
		{
			pResponse->strContent += "<script type=\"text/javascript\">\n";
			pResponse->strContent += "<!--\n";
			pResponse->strContent += "document.postatalk.talk.focus();\n";
			pResponse->strContent += "var talkData = '" + UTIL_RemoveHTML3( cstrTalk ) + "';\n";
			pResponse->strContent += "document.postatalk.talk.value = talkData.stripX( );\n";
			pResponse->strContent += "charleft(document.postatalk,'talk_left','submit_talk');\n";
			pResponse->strContent += "//-->\n";
			pResponse->strContent += "</script>\n\n";
		}

		// Output common HTML tail
		HTML_Common_End( pRequest, pResponse, btv, NOT_INDEX, string( CSS_TALK ) );
	}
	else
	{
		// Not authorised

		// Output common HTML head
		HTML_Common_Begin(  pRequest, pResponse, gmapLANG_CFG["talk_page"], string( CSS_TALK ), string( ), NOT_INDEX, CODE_401 );

		// Output common HTML tail
		HTML_Common_End( pRequest, pResponse, btv, NOT_INDEX, string( CSS_TALK ) );
	}
}

void CTracker :: serverResponseTalkPOST( struct request_t *pRequest, struct response_t *pResponse, CAtomList *pPost )
{
	// Set the start time
	const struct bnbttv btv( UTIL_CurrentTime( ) );

	// Verify that the IP is permitted to access the tracker
	if( m_ucIPBanMode != 0 )
		if( IsIPBanned( pRequest, pResponse, btv, gmapLANG_CFG["talk_page"], string( CSS_TALK ), NOT_INDEX ) )
			return;
	
	if( pRequest->user.ucAccess & m_ucAccessComments )
	{
		string strTalk = string( );
		string strReply = string( );
		string strReplyTo = string( );
		string strReplyToID = string( );
		string strRT = string( );
		string strRTTo = string( );
		string strRTToID = string( );
		string cstrID = string( );
		string cstrShow = string( );
		string cstrPage = string( );
		string cstrPerPage = string( );
		
		if( pPost )
		{
			// Initialise segment dictionary
			CAtomDicti *pSegment = 0;
			CAtom *pDisposition = 0;
			CAtom *pData = 0;
			CAtom *pName = 0;
			CAtom *pFile = 0;
			// Get the segments from the post
			vector<CAtom *> vecSegments = pPost->getValue( );

			// Loop through the segments

			for( unsigned long ulCount = 0; ulCount < vecSegments.size( ); ulCount++ )
			{
				// Is the segment a dictionary?
				if( vecSegments[ulCount]->isDicti( ) )
				{
					// Get the segment dictionary
					pSegment = (CAtomDicti *)vecSegments[ulCount];
					// Get the disposition and the data from the segment dictionary
					pDisposition = pSegment->getItem( "disposition" );
					pData = pSegment->getItem( "data" );

					// Did we get a disposition that is a dictionary and has data?
					if( pDisposition && pDisposition->isDicti( ) && pData )
					{
						// Get the content name from the disposition
						pName = ( (CAtomDicti *)pDisposition )->getItem( "name" );

						// Did we get a content name?
						if( pName )
						{
							// What is the content name to be tested?
							string strName = pName->toString( );
							
							if( strName == "talk" )
								strTalk = pData->toString( );
							else if( strName == "reply" )
								strReply = pData->toString( );
							else if( strName == "replyto" )
								strReplyToID = pData->toString( );
							else if( strName == "rt" )
								strRT = pData->toString( );
							else if( strName == "rtto" )
								strRTToID = pData->toString( );
							else if( strName == "show" )
								cstrShow = pData->toString( );
							else if( strName == "page" )
								cstrPage = pData->toString( );
							else if( strName == "per_page" )
								cstrPerPage = pData->toString( );
						}
						else
						{
							// Output common HTML head
							HTML_Common_Begin(  pRequest, pResponse, gmapLANG_CFG["talk_page"], string( CSS_TALK ), string( ), NOT_INDEX, CODE_400 );

							// failed
							pResponse->strContent += "<p class=\"failed\">" + gmapLANG_CFG["failed"] + "</p>\n";
							// Signal a bad request
							pResponse->strContent += "<p class=\"body_upload\">400 " + gmapLANG_CFG["server_response_400"] + "</p>\n";

							// Output common HTML tail
							HTML_Common_End( pRequest, pResponse, btv, NOT_INDEX, string( CSS_TALK ) );

							if( gbDebug )
								UTIL_LogPrint( "Upload Warning - Bad request (no torrent name)\n" );

							return;
						}
					}
				}
			}
		}
		else
		{
			// Output common HTML head
			HTML_Common_Begin(  pRequest, pResponse, gmapLANG_CFG["talk_page"], string( CSS_TALK ), string( ), NOT_INDEX, CODE_400 );

			// failed
			pResponse->strContent += "<p class=\"failed\">" + gmapLANG_CFG["failed"] + "</p>\n";
			// Signal a bad request
			pResponse->strContent += "<p class=\"body_upload\">400 " + gmapLANG_CFG["server_response_400"] + "</p>\n";

			// Output common HTML tail
			HTML_Common_End( pRequest, pResponse, btv, NOT_INDEX, string( CSS_TALK ) );

			if( gbDebug )
				UTIL_LogPrint( "Upload Warning - Bad request (no post received)\n" );

			return;
		}
		
		vector< pair< string, string > > vecParams;
		vecParams.reserve(64);
		string strJoined = string( );
		string strJoinedHTML = string( );

//		vecParams.push_back( pair<string, string>( string( "uid" ), strUID ) );
		vecParams.push_back( pair<string, string>( string( "show" ), cstrShow ) );
//		vecParams.push_back( pair<string, string>( string( "page" ), cstrPage ) );
		vecParams.push_back( pair<string, string>( string( "per_page" ), cstrPerPage ) );
		
		strJoinedHTML = UTIL_HTMLJoin( vecParams, string( "?" ), string( "&" ), string( "=" ) );
		strJoined = UTIL_RemoveHTML( strJoinedHTML );
		
		bool bNoComment = false;
		
		if( ( !bNoComment || pRequest->user.ucAccess & m_ucAccessCommentsAlways ) )
		{
// 			strTalk = strTalk.substr( 0, m_uiCommentLength );

			if( strTalk.empty( ) )
			{
				HTML_Common_Begin(  pRequest, pResponse, gmapLANG_CFG["talk_page"], string( CSS_TALK ), string( ), NOT_INDEX, CODE_200 );
				//You must fill in all the fields.

				pResponse->strContent += "<p class=\"fill_all\">" + UTIL_Xsprintf( gmapLANG_CFG["talk_fill_warning"].c_str( ), string( "<a title=\"" + gmapLANG_CFG["talk"] + "\" href=\"" + RESPONSE_STR_TALK_HTML + strJoined + "\">" ).c_str( ), "</a>" ) + "</p>\n";

				// Output common HTML tail
				HTML_Common_End( pRequest, pResponse, btv, NOT_INDEX, string( CSS_TALK ) );

				return;
			}
			
			string strTalkStore( strTalk );
			
			set< pair< string, string > > setRef;
			
			string :: size_type iStart = 0;
			string :: size_type iEnd = 0;
			string :: size_type iEnd1 = 0;
			string :: size_type iLength = 0;
			
			iStart = strTalk.find( "@" );
			while( iStart != string :: npos )
			{
				iLength = 0;
				iEnd = strTalk.find_first_of( " :", iStart );
				if( iEnd != string :: npos )
					iLength = 1;
				iEnd1 = strTalk.find( "：", iStart );
				if( iEnd1 < iEnd )
				{
					iEnd = iEnd1;
					iLength = string( "：" ).size( );
				}
				
				string strUsername = strTalk.substr( iStart + 1, iEnd - iStart - 1 );

				if( strUsername.size( ) > 0 && strUsername.size( ) <= 16 && ( iStart != 0 || iEnd != string :: npos ) )
				{
					CMySQLQuery *pQueryUser = new CMySQLQuery( "SELECT buid,busername FROM users WHERE busername=\'" + UTIL_StringToMySQL( strUsername ) + "\'" );

					vector<string> vecQueryUser;

					vecQueryUser.reserve(2);

					vecQueryUser = pQueryUser->nextRow( );

					delete pQueryUser;

					if( vecQueryUser.size( ) == 2 && !vecQueryUser[0].empty( ) )
					{
						if( vecQueryUser[0] == strReplyToID )
							strReplyTo = vecQueryUser[1];
						if( vecQueryUser[0] == strRTToID )
							strRTTo = vecQueryUser[1];
						
						setRef.insert( pair< string, string >( strUsername, vecQueryUser[0] ) );
						
						strTalk.replace( iStart + 1, iEnd - iStart - 1 + iLength, "#" + vecQueryUser[0] + "# " );
						
						iStart = strTalk.find( "@", iStart + 1 + vecQueryUser[0].size( ) + 2 );
					}
					else
						iStart = strTalk.find( "@", iStart + 1 );
				}
				else
					iStart = strTalk.find( "@", iStart + 1 );
			}
			
			set<string> setTag;
			
//			string :: size_type iStart = 0;
//			string :: size_type iEnd = 0;
			
			iStart = strTalkStore.find( "#" );
			while( iStart != string :: npos )
			{
				iEnd = strTalkStore.find( "#", iStart + 1 );
				
				if( iEnd != string :: npos )
				{
					string strTag = strTalkStore.substr( iStart + 1, iEnd - iStart - 1 );
					
					if( strTag.size( ) > 0 && strTag.size( ) <= 40 )
					{
						if( strTag.find_first_of( "\r\n" ) == string :: npos )
						{
							setTag.insert( strTag );

							iStart = strTalkStore.find( "#", iEnd + 1 );
						}
						else
							iStart = strTalkStore.find( "#", iEnd + 1 );
					}
					else
						iStart = strTalkStore.find( "#", iEnd + 1 );
				}
				else
					iStart = strTalkStore.find( "#", iEnd );
			}
			
			iStart = strTalk.find_first_not_of( " " );
			iEnd = strTalk.find_last_not_of( " " );
			strTalk = strTalk.substr( iStart, iEnd - iStart + 1 );
			
			iStart = strTalkStore.find_first_not_of( " " );
			iEnd = strTalkStore.find_last_not_of( " " );
			strTalkStore = strTalkStore.substr( iStart, iEnd - iStart + 1 );
			
			string strQuery = "INSERT INTO talk (";
			strQuery += "busername,buid,bip,bposted,btalk,btalkstore";
			if( !strReplyTo.empty( ) )
				strQuery += ",breply,breplyto,breplytoid";
			if( !strRTTo.empty( ) )
				strQuery += ",brt,brtto,brttoid";
			strQuery += ") VALUES(";
			strQuery += "\'" + UTIL_StringToMySQL( pRequest->user.strLogin );
			strQuery += "\'," + pRequest->user.strUID;
			strQuery += ",\'" + UTIL_StringToMySQL( pRequest->strIP );
			strQuery += "\',NOW()";
			strQuery += ",\'" + UTIL_StringToMySQL( strTalk ) + "\'";
			strQuery += ",\'" + UTIL_StringToMySQL( strTalkStore ) + "\'";
			if( !strReplyTo.empty( ) )
				strQuery += "," + strReply + ",'" + UTIL_StringToMySQL( strReplyTo ) + "'," + strReplyToID;
			if( !strRTTo.empty( ) )
				strQuery += "," + strRT + ",'" + UTIL_StringToMySQL( strRTTo ) + "'," + strRTToID;
			strQuery += ")";
			
			CMySQLQuery *pQuery = new CMySQLQuery( strQuery );
			
			unsigned long ulLast = pQuery->lastInsertID( );
			
			delete pQuery;
			
			string strPosted = string( );
			
			if( ulLast > 0 )
			{
				CMySQLQuery *pQueryTalk = new CMySQLQuery( "SELECT bposted FROM talk WHERE bid=" + CAtomLong( ulLast ).toString( ) );
			
				vector<string> vecQueryTalk;
	
				vecQueryTalk.reserve(1);

				vecQueryTalk = pQueryTalk->nextRow( );

				delete pQueryTalk;
			
				if( vecQueryTalk.size( ) == 1 )
					strPosted = vecQueryTalk[0];
				
				for ( set< pair< string, string > > :: iterator it = setRef.begin( ); it != setRef.end( ); it++ )
				{
					CMySQLQuery mq01( "INSERT INTO talkref (busername,buid,breferid,brefid,bposted) VALUES('" + UTIL_StringToMySQL( (*it).first ) + "'," + (*it).second + "," + pRequest->user.strUID + "," + CAtomLong( ulLast ).toString( ) + ",'" + UTIL_StringToMySQL( strPosted ) + "')" );
				
					CMySQLQuery mq02( "UPDATE users SET btalkref=btalkref+1 WHERE buid=" + (*it).second );
				}
				
				for ( set<string> :: iterator it = setTag.begin( ); it != setTag.end( ); it++ )
				{
					CMySQLQuery mq01( "INSERT INTO talktag (btag,bid,bposted) VALUES('" + UTIL_StringToMySQL( *it ) + "'," + CAtomLong( ulLast ).toString( ) + ",'" + UTIL_StringToMySQL( strPosted ) + "')" );
				}
				

				CMySQLQuery mq03( "INSERT INTO talkhome (buid,btalkid,bposted) VALUES(" + pRequest->user.strUID + "," + CAtomLong( ulLast ).toString( ) + ",'" + UTIL_StringToMySQL( strPosted ) + "')" );
				
				CMySQLQuery *pQueryFriend = new CMySQLQuery( "SELECT buid FROM friends WHERE bfriendid=" + pRequest->user.strUID );
			
				vector<string> vecQueryFriend;
	
				vecQueryFriend.reserve(1);

				vecQueryFriend = pQueryFriend->nextRow( );
				
				while( vecQueryFriend.size( ) == 1 )
				{
					if( !vecQueryFriend[0].empty( ) )
					{
						CMySQLQuery mq04( "INSERT INTO talkhome (buid,bfriendid,btalkid,bposted) VALUES(" + vecQueryFriend[0] + "," + pRequest->user.strUID + "," + CAtomLong( ulLast ).toString( ) + ",'" + UTIL_StringToMySQL( strPosted ) + "')" );
						CMySQLQuery mq05( "UPDATE users SET btalk=btalk+1 WHERE buid=" + vecQueryFriend[0] );
					}
						
					vecQueryFriend = pQueryFriend->nextRow( );
				}
		
				delete pQueryFriend;
			}
			
			// Your comment has been posted.
			
			HTML_Common_Begin(  pRequest, pResponse, gmapLANG_CFG["talk_page"], string( CSS_TALK ), string( TALK_HTML + strJoinedHTML ), NOT_INDEX, CODE_200 );
//			pResponse->strContent += "<p class=\"comments_posted\">" + UTIL_Xsprintf( gmapLANG_CFG["comments_posted"].c_str( ), string( "<a title=\"" + gmapLANG_CFG["comments"] + "\" href=\"" + RESPONSE_STR_TALK_HTML + strJoined + "\">" ).c_str( ), "</a>" ) + "</p>\n";

			// Output common HTML tail
			HTML_Common_End( pRequest, pResponse, btv, NOT_INDEX, string( CSS_TALK ) );

			return;
		}
	}
}

const string CTracker :: TransferMentions( const string &cstrTalk, const string &cstrTalkID )
{
	string strTalk = UTIL_RemoveHTML( cstrTalk );
	string :: size_type iStart = 0;
	string :: size_type iEnd = 0;
	string :: size_type iEnd1 = 0;
	
	iStart = strTalk.find( "@#" );
	while( iStart != string :: npos )
	{
		iEnd = strTalk.find( "#", iStart + 2 );
		if( iEnd != string :: npos )
		{
			string strUserLink = string( );
			
			string strUID = strTalk.substr( iStart + 2, iEnd - iStart - 2 );
			
			if( strUID.size( ) > 0  )
			{
				if( strUID.find_first_not_of( "1234567890" ) == string :: npos )
				{
					CMySQLQuery *pQueryRef = new CMySQLQuery( "SELECT busername FROM talkref WHERE buid=" + strUID + " AND brefid=" + cstrTalkID );

					vector<string> vecQueryRef;
	
					vecQueryRef.reserve(1);

					vecQueryRef = pQueryRef->nextRow( );
	
					delete pQueryRef;
				
					if( vecQueryRef.size( ) == 1 && !vecQueryRef[0].empty( ) )
					{
						string strUsername = vecQueryRef[0];
						
						strUserLink = getUserLinkTalk( strUID, strUsername );
						
						strTalk.replace( iStart + 1, iEnd - iStart, strUserLink );
						
						iStart = strTalk.find( "@#", iStart + 1 + strUserLink.size( ) );
					
//						CMySQLQuery *pQuery = new CMySQLQuery( "SELECT baccess,bgroup,buploaded,bdownloaded,UNIX_TIMESTAMP(bwarned) FROM users WHERE buid=" + strUID );

//						vector<string> vecQuery;
//	
//						vecQuery.reserve(5);

//						vecQuery = pQuery->nextRow( );
//	
//						delete pQuery;
//	

//						if( vecQuery.size( ) == 5 )
//						{
//							int64 iUploaded = 0, iDownloaded = 0;
//							float flShareRatio = 0;
//		
//							iUploaded = UTIL_StringTo64( vecQuery[2].c_str( ) );
//							iDownloaded = UTIL_StringTo64( vecQuery[3].c_str( ) );

//							if( iDownloaded == 0 )
//							{
//								if( iUploaded == 0 )
//									flShareRatio = 0;
//								else
//									flShareRatio = -1;
//							}
//							else
//								flShareRatio = (float)iUploaded / (float)iDownloaded;
//		
//							bool bShareRatioWarned = checkShareRatio( iDownloaded, flShareRatio );
//							string strClass = UTIL_UserClass( (unsigned char)atoi( vecQuery[0].c_str( ) ), (unsigned char)atoi( vecQuery[1].c_str( ) ) );
//		
//							strUserLink += "<a class=\"";
//		
//							if( bShareRatioWarned && strClass == gmapLANG_CFG["class_member"] )
//								strUserLink += "share_warned";
//							else
//								strUserLink += strClass;
//		
//							strUserLink += "\" href=\"" + RESPONSE_STR_TALK_HTML + "?uid=" + strUID + "\">" + UTIL_RemoveHTML( strUsername ) + "</a>";

//							if( m_bRatioRestrict && checkShareRatio( iDownloaded, flShareRatio ) )
//								strUserLink += "<img title=\"" + gmapLANG_CFG["user_shareratio_warned"] + "\" src=\"files/warned3.gif\">";
//							if( vecQuery[4] != "0" )
//								strUserLink += "<img title=\"" + gmapLANG_CFG["user_warned"] + "\" src=\"files/warned.gif\">";
//							
//							strTalk.replace( iStart + 1, iEnd - iStart, strUserLink );
//					
//							iStart = strTalk.find( "@", iStart + 1 + strUserLink.size( ) );
//						}
//						else
//							iStart = strTalk.find( "@", iStart + 1 );
					}
					else
						iStart = strTalk.find( "@#", iStart + 2 );
				}
				else
					iStart = strTalk.find( "@#", iStart + 2 );
			}
			else
				iStart = strTalk.find( "@#", iStart + 2 );
		}
		else
			iStart = strTalk.find( "@#", iEnd );
	}
	
	string :: size_type iLength = gmapLANG_CFG["torrent"].size( );
	iStart = strTalk.find( "#" );
	while( iStart != string :: npos )
	{
		iEnd = strTalk.find( "#", iStart + 1 );
			
		if( iEnd != string :: npos )
		{
			string strTopic = strTalk.substr( iStart, iEnd - iStart + 1 );
			string strTag = strTalk.substr( iStart + 1, iEnd - iStart - 1 );
			
			if( strTag.size( ) > 0 && strTag.size( ) <= 40 )
			{
				string strID = string( );
				string strFullLink = string( );

				if( strTag.find( "<br>" ) == string :: npos )
				{
					if( strTag.find( gmapLANG_CFG["torrent"] ) == 0 )
					{
						strID = strTag.substr( iLength, iEnd - iStart - 1 - iLength );
						if( !strID.empty( ) && strID.find_first_not_of( "1234567890" ) == string :: npos )
							strFullLink = "<a class=\"talk_torrent_link\" href=\"javascript: ;\" onClick=\"javascript: get_torrent(this.parentNode.id,'" + strID + "');\">" + strTopic + "</a>";
						else
							strFullLink = "<a class=\"talk_tag\" href=\"" + RESPONSE_STR_TALK_HTML + "?tag=" + UTIL_StringToEscaped( strTag ) + "\">" + strTopic + "</a>";
					}
					else
					{
						strFullLink = "<a class=\"talk_tag\" href=\"" + RESPONSE_STR_TALK_HTML + "?tag=" + UTIL_StringToEscaped( strTag ) + "\">" + strTopic + "</a>";
					}
					strTalk.replace( iStart, iEnd - iStart + 1, strFullLink );

					iStart = strTalk.find( "#", iStart + strFullLink.size( ) );
				}
				else
					iStart = strTalk.find( "#", iEnd + 1 );
			}
			else
				iStart = strTalk.find( "#", iEnd + 1 );
		}
		else
			iStart = strTalk.find( "#", iEnd );
	}
	
//	iStart = strTalk.find( "#" );
//	while( iStart != string :: npos )
//	{
//		iEnd = strTalk.find( "#", iStart + 1 );
//			
//		if( iEnd - iStart - 1 > 0 )
//		{
//			string strTag = strTalk.substr( iStart, iEnd - iStart + 1 );
//			string strID = strTag.substr( 1, iEnd - iStart - 1 );

//			string strFullLink = "<a class=\"talk_link\" href=\"" + RESPONSE_STR_TALK_HTML + "?tag=" + strID + "\">" + strTag + "</a>";

//			strTalk.replace( iStart, iEnd - iStart + 1, strFullLink );
//			
//			iStart = UTIL_ToLower( strTalk ).find( "#", iStart + strFullLink.size( ) );
//		}
//		else
//			iStart = UTIL_ToLower( strTalk ).find( "#", iEnd );
//	}

	iStart = UTIL_ToLower( strTalk ).find( "http://" );
	while( iStart != string :: npos )
	{
		iEnd = UTIL_ToLower( strTalk ).find_first_not_of( "abcdefghijklmnopqrstuvwxyz1234567890./", iStart + 7 );
		
		string strLink = strTalk.substr( iStart, iEnd - iStart );
		
		if( strLink.size( ) > 7 )
		{
			string :: size_type iSlash = strLink.find( "/", iStart + 7 );
			string strDomain = strLink.substr( 7, iSlash - 7 );
			bool bURL = true;

			if( strDomain.size( ) > 0 )
			{
				vector<string> vecDomain;
				vecDomain.reserve(64);
				
				vecDomain = UTIL_SplitToVectorStrict( strDomain, "." );

				if( vecDomain.size( ) > 1 )
				{
					for( vector<string> :: iterator ulKey = vecDomain.begin( ); ulKey != vecDomain.end( ); ulKey++ )
					{
						if( (*ulKey).size( ) == 0 )
						{
							bURL = false;
							break;
						}
						if( ( ulKey + 1 ) == vecDomain.end( ) && ( (*ulKey).size( ) < 2 || (*ulKey).size( ) > 4 ) )
						{
							bURL = false;
							break;
						}
					}
				}
				else
					bURL = false;
			}
			
			if( bURL )
			{
				string strFullLink = "<a class=\"talk_link\" target=\"_blank\" href=\"" + strLink + "\">" + strLink + "</a>";
			
				strTalk.replace( iStart, iEnd - iStart, strFullLink );
			
				iStart = UTIL_ToLower( strTalk ).find( "http://", iStart + strFullLink.size( ) );
			}
			else
				iStart = UTIL_ToLower( strTalk ).find( "http://", iEnd );

		}
		else
			iStart = UTIL_ToLower( strTalk ).find( "http://", iEnd );
	}
	
	
	return strTalk;
}

const string CTracker :: GenerateTalk( const vector<string> &vecQuery, const unsigned char cucAccess, const string &cstrUID, const string &cstrJoined, bool bTalker, bool bFunc )
{
	string strReturn = string( );
//	string strTorrentReturn = string( );

	string strID = string( );
	string strName = string( );
	string strNameID = string( );
	string strTime = string( );
	string strComText = string( );
	string strReplyCom = string( );
	string strReplyID = string( );
	string strReplyTo = string( );
	string strReplyToID = string( );
	string strRTID = string( );
	string strRTTo = string( );
	string strRTToID = string( );

	strID = vecQuery[0];

	if( !vecQuery[1].empty( ) )
		strName = vecQuery[1];

	if( !vecQuery[2].empty( ) )
		strNameID = vecQuery[2];

	strTime = vecQuery[3];
	strComText = vecQuery[4];
	strReplyCom = vecQuery[5];
	strReplyID = vecQuery[6];
	strReplyTo = vecQuery[7];
	strReplyToID = vecQuery[8];
	strRTID = vecQuery[9];
	strRTTo = vecQuery[10];
	strRTToID = vecQuery[11];

	if( strTime.empty( ) )
		strTime = gmapLANG_CFG["unknown"];

	//
	// body
	//
	strReturn += "<tr class=\"talk_body\"";
	if( bFunc )
		strReturn += " onMouseOver=\"javascript: display('func" + strID + "');\" onMouseOut=\"javascript: hide('func" + strID + "');\"";
	strReturn += ">\n";
	strReturn += "<span style=\"display: none\" id=\"reply" + strID + "\">@" + UTIL_RemoveHTML( strName ) + " </span>\n";
	strReturn += "<span style=\"display: none\" id=\"rt" + strID + "\">" + UTIL_Xsprintf( gmapLANG_CFG["talk_rt_content"].c_str( ), UTIL_RemoveHTML( strName ).c_str( ), UTIL_RemoveHTML3( strReplyCom ).c_str( ) ) + "</span>\n";
	strReturn += "<td class=\"talk_body\">\n<div id=\"talk" + strID + "\">";
	if( bTalker )
	{
		string strUserLink = getUserLinkTalk( strNameID, strName );

		if( strUserLink.empty( ) )
			strUserLink = gmapLANG_CFG["unknown"];
		strReturn += strUserLink + " ";
	}
	strReturn += TransferMentions( strComText, strID );
	strReturn += "<br><a class=\"talk_time\" href=\"" + RESPONSE_STR_TALK_HTML + "?id=" + strID + "\">" + strTime + "</a>";
	if( !strReplyTo.empty( ) )
	{
		if( bFunc )
			strReturn += " " + UTIL_Xsprintf( gmapLANG_CFG["talk_reply_to"].c_str( ), string( "<a class=\"talk_reply_link\" href=\"javascript: ;\" onClick=\"get_reply(this.parentNode.id,'" + strReplyID + "');\">" ).c_str( ), UTIL_RemoveHTML( strReplyTo ).c_str( ), "</a>" );
//			strReturn += " " + UTIL_Xsprintf( gmapLANG_CFG["talk_reply_to"].c_str( ), string( "<a class=\"talk_reply_link\" href=\"javascript: ;\" onClick=\"get_reply('" + strID + "','" + strReplyID + "');\">" ).c_str( ), UTIL_RemoveHTML( strReplyTo ).c_str( ), "</a>" );
//			strReturn += " " + UTIL_Xsprintf( gmapLANG_CFG["talk_reply_to"].c_str( ), string( "<a class=\"talk_reply_link\" href=\"" + RESPONSE_STR_TALK_HTML + "?id=" + strReplyID + "\">" ).c_str( ), UTIL_RemoveHTML( strReplyTo ).c_str( ), "</a>" );
		else
			strReturn += " " + UTIL_Xsprintf( gmapLANG_CFG["talk_reply_to"].c_str( ), string( "<a class=\"talk_reply_link\" href=\"" + RESPONSE_STR_TALK_HTML + "?id=" + strReplyID + "\">" ).c_str( ), UTIL_RemoveHTML( strReplyTo ).c_str( ), "</a>" );
	}

	if( !strRTTo.empty( ) )
	{
		if( bFunc )
			strReturn += " " + UTIL_Xsprintf( gmapLANG_CFG["talk_rt_to"].c_str( ), string( "<a class=\"talk_rt_link\" href=\"javascript: ;\" onClick=\"get_rt(this.parentNode.id,'" + strRTID + "');\">" ).c_str( ), UTIL_RemoveHTML( strRTTo ).c_str( ), "</a>" );
//			strReturn += " " + UTIL_Xsprintf( gmapLANG_CFG["talk_rt_to"].c_str( ), string( "<a class=\"talk_rt_link\" href=\"javascript: ;\" onClick=\"get_rt('" + strID + "','" + strRTID + "');\">" ).c_str( ), UTIL_RemoveHTML( strRTTo ).c_str( ), "</a>" );
//			strReturn += " " + UTIL_Xsprintf( gmapLANG_CFG["talk_rt_to"].c_str( ), string( "<a class=\"talk_rt_link\" href=\"" + RESPONSE_STR_TALK_HTML + "?id=" + strRTID + "\">" ).c_str( ), UTIL_RemoveHTML( strRTTo ).c_str( ), "</a>" );
		else

			strReturn += " " + UTIL_Xsprintf( gmapLANG_CFG["talk_rt_to"].c_str( ), string( "<a class=\"talk_rt_link\" href=\"" + RESPONSE_STR_TALK_HTML + "?id=" + strRTID + "\">" ).c_str( ), UTIL_RemoveHTML( strRTTo ).c_str( ), "</a>" );
	}
	strReturn += "</div>\n</td>\n";

	if( bFunc )
	{
		strReturn += "<td class=\"talk_function\">";
		strReturn += "<div style=\"visibility: hidden\" id=\"func" + strID + "\">";
		if( ( cucAccess & m_ucAccessComments ) && !cstrUID.empty( ) && ( cstrUID != strNameID ) )
		{
			strReturn += "[<a class=\"talk_reply\" title=\"" + gmapLANG_CFG["talk_reply"] + "\" href=\"javascript:;\" onclick=\"javascript: reply( '" + strID + "','" + strNameID + "', 'postForm', 'talkarea', 'inputReply' );\">" + gmapLANG_CFG["talk_reply"] + "</a>]";
			strReturn += "<br>[<a class=\"talk_rt\" title=\"" + gmapLANG_CFG["talk_rt"] + "\" href=\"javascript:;\" onclick=\"javascript: rt( '" + strID + "','" + strNameID + "', 'postForm', 'talkarea', 'inputReply', 'inputRT' );\">" + gmapLANG_CFG["talk_rt"] + "</a>]";
		}
		if( !cstrUID.empty( ) && cstrUID == strNameID )
		{
			strReturn += "[<a class=\"talk_delete\" title=\"" + gmapLANG_CFG["delete"] + "\" href=\"javascript: ;\" onClick=\"javascript: if( confirm('" + gmapLANG_CFG["talk_delete_q"] + "') ) load('div','divTalk','" + RESPONSE_STR_TALK_HTML + cstrJoined;
			if( cstrJoined.empty( ) )
				strReturn += "?";
			else
				strReturn += "&amp;";
			strReturn += "del=" + strID;
			strReturn += "');\">" + gmapLANG_CFG["delete"] + "</a>]";
		}
		strReturn += "</div>";
		strReturn += "</td>\n";
	}
	strReturn += "</tr>\n";
	
//	strReturn += strTorrentReturn + "</tr>\n";
//	strReturn += "<div style=\"display: none;\" id=\"return" + strID + "\">\n" + strTorrentReturn + "</div>\n";
	

	return strReturn;
}
