##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "FormMail" do
author "Andrew Horton"
version "0.1"
description "Common form email script.

FormMail is a Perl script written by Matt Wright to send mail with sendmail from the cgi-gateway. Early version didn't have a referer check. New versions could be misconfigured. Spammers are known to hunt them down (by means of cgi-scanning) and abuse them for their own evil purposes if the admin forgot to check the settings.http://www.securityfocus.com/bid/3954/discussion/"

examples %w| http://dominio.netfirms.com/cgi-bin/FormMail.pl http://eng.yale.edu/cgi-bin/classes/ee101/FormMail.pl http://belmorenursesbureau.com.au/cgi-bin/FormMail.pl http://www.ldb.org/iphw/iphw_f.htm http://www.fabius-pompey.com/fpsurvey.htm|

#  
  
  
  
matches [

{:name=>"This form was suplied by", 
:probability=>100,
:text=>'This form was suplied by <A href="http://www.worldwidemart.com/scripts/formmail.shtml"' },

{:name=>"FormMail link", 
:probability=>100,
:text=>'<a href="http://www.worldwidemart.com/scripts/formmail.shtml">FormMail</a>' },

{:name=>"copyright", 
:probability=>75,
:regexp=>/&copy; 19[\d]+ -[\d]+ Matt Wright<BR>/ },

{:name=>"title", 
:probability=>75,
:text=>'<title>FormMail ' },

{:name=>"a free product of", 
:probability=>50,
:text=>'A Free Product of <a href="http://www.worldwidemart.com/scripts/">Matt\'s Script Archive,' }, #'

{:name=>"Formmail th", 
:probability=>100,
:text=>"<tr><th><font size=+2>FormMail</font></th></tr>" },

{:name=>"FormMail.pl", 
:probability=>100,
:ghdb=>"inurl:/FormMail.pl" },

{:name=>"version2", 
:probability=>100,
:version=>"1.9",
:regexp=>/Copyright 1995 - 2001 Matt Wright<br>[\s]+Version 1.9/ }
]


def passive
        m=[]
        
        if @body =~ /FormMail<\/a>[\s]+V[\d\.]+/i
                version=@body.scan(/FormMail<\/a>[\s]+V([\d\.]+)/i)[0][0]
                m << {:probability=>100,:name=>"version",:version=>version} 
        end
        
        m
end



end

