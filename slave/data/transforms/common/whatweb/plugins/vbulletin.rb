##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "VBulletin" do
author "Andrew Horton"
version "0.1"
description "VBulletin is a PHP forum.  http://johnny.ihackstuff.com/ghdb/?function=detail&id=336"
examples %w|forums.photobucket.com www.hotscripts.com/forums/ www.hardforum.com/ scannerbuff.net/forums/index.php http://mb.winneronline.com www.reggaesound.com/upload/ www.digitalphotopros.com/forums/ www.computerforumz.com/ www.mountainbuzz.com/gallery www.alhesbah.org/v www.diabetesforums.com forums.somethingawful.com|

matches [
{:name=>"\"Powered by vBulletin\" inurl:newreply.php",
:probability=>100,
:ghdb=>'"Powered by vBulletin" inurl:newreply.php'},

{:name=>"meta generator tag",
:probability=>100,
:regexp=>/<meta name="generator" content="vBulletin/},

{:name=>"CSS comments",
:probability=>75,
:regexp=>/\* vBulletin [0-9a-z.]+ CSS/},

{:name=>"vbulletin_global.js",
:probability=>100,
:regexp=>/<script type="text\/javascript" src="[a-z0-9.\/]*vbulletin_global.js/},

{:name=>"vb_bullet.gif",
:probability=>75,
:regexp=>/vb_bullet.gif"/}, #" comment for gedit syntax hilighting

{:name=>"powered by",
:probability=>100,
:regexp=>/Powered by(:)? vBulletin(&reg;)? Version/},

{:name=>"copyright Jelsoft",
:probability=>50,
:regexp=>/Copyright &copy;2000 - [0-9]+, Jelsoft Enterprises Ltd./}

]

 
# <meta name="generator" content="vBulletin 3.8.3" />
# <style type="text/css" id="vbulletin_css">
# <script type="text/javascript" src="clientscript/vbulletin_menu.js?v=383"></script>
# <script type="text/javascript" src="clientscript/vbulletin_md5.js?v=383"></script>
# <td class="vbmenu_control">
# // Main vBulletin Javascript Initialization


# <meta name="generator" content="vBulletin 3.7.3" />
# <meta name="generator" content="vBulletin 3.8.2" />
# <meta name="generator" content="vBulletin 3.0.0" />

# * vBulletin 3.7.3 CSS
# * vBulletin 3.8.2 CSS

# <script type="text/javascript" src="clientscript/vbulletin_global.js?v=373"></script>
# <input type="hidden" name="vb_login_md5password" />

# vb_bullet.gif

# Powered by: vBulletin Version 2.0.3<br>
# Powered by: vBulletin Version 2.3.2<br> 
# Powered by vBulletin&reg; Version 3.7.3<br />Copyright &copy;2000 - 2009, Jelsoft Enterprises Ltd.
# vBulletin&reg; Copyright &copy;2000 - 2009, Jelsoft Enterprises Ltd.
# Copyright &copy;2000 - 2009, Jelsoft Enterprises Ltd.
# Powered by: vBulletin Version 3.0.0<br />Copyright &copy;2000 - 2009, Jelsoft Enterprises Ltd.


def passive
	m=[]
	if @body =~ /Powered by(:)? vBulletin(&reg;)? Version ([0-9a-z.]+)/
		version=@body.scan(/Powered by(:)? vBulletin(&reg;)? Version ([0-9a-z.]+)/)[0][2]
		m << {:probability=>100,:name=>"powered by copyright footer",:version=>version}
	end
	
	if @body =~ /<meta name="generator" content="vBulletin ([0-9a-z.]+)" \/>/
		version=@body.scan(/<meta name="generator" content="vBulletin ([0-9a-z.]+)" \/>/)[0][0]
		m << {:probability=>100,:name=>"meta generator tag",:version=>version}
	end
	
	if @body =~ /\* vBulletin ([0-9a-z.]+) CSS/
		version=@body.scan(/\* vBulletin ([0-9a-z.]+) CSS/)[0][0]
		m << {:probability=>100,:name=>"css comments",:version=>version}
	end

	m
end

end

