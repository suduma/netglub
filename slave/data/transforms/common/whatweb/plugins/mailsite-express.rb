##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "MailSiteExpress" do
author "Andrew Horton"
version "0.1"
description "Webmail in ASP. Versions < 6.1.2 insecure http://marc.info/?l=bugtraq&m=113053680631151&w=2   Homepage: http://www.mailsite.com/products/express-webmail-server.asp"
examples %w|202.37.232.199 webmail.oasisonline.com www.itsmail.ca/its/ mail.roar.net.nz/express/ www.cmda.com.ve/content/correo_content.html|


# <!-- This software is copyright 1999, 2000 Rockliffe systems, Inc. -->
# <!-- default.asp ver. 4.5.6 -->
# <title>MailSite Express</title>
# <font face="arial" size="1">MailSite Express <br> version 4.5.6</font>

# version 6
# <b>MailSite <em>Express</em> Login</b>
# onSubmit="OpenExpress(document.ExpressLogin)"

matches [
{:name=>"title", 
:probability=>100,
:text=>'<title>MailSite Express</title>' },

{:name=>"powered by", 
:probability=>100,
:regexp=>/<font face="arial" size="1">MailSite Express <br> version [0-9\.]+<\/font>/ },

{:name=>"Copyright in comments",
:probability=>50,
:regexp=>/<!-- This software is copyright [0-9 ,\.]+ Rockliffe systems, Inc. -->/ },

{:name=>"MailSite Express Login",
:probability=>100,
:text=>'<b>MailSite <em>Express</em> Login</b>' },

{:name=>"javascript",
:probability=>100,
:text=>'onSubmit="OpenExpress(document.ExpressLogin)"' }

]

end

