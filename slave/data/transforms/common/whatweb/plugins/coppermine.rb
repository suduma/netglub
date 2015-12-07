##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "Coppermine" do
author "Andrew Horton"
version "0.1"
description "PHP & MySQL Photo Gallery homepage: www.coppermine-gallery.net"
examples %w| http://www.langlandsurf.com/gallery/index.php http://www.botanyphotos.net/ http://www.pngtrekkingadventures.com/gallery/ 
ralphbakshi.com/gallery makro.arachiusz.pl www.sabre-roads.org.uk/gallery www.desertdave.com www.akronaeros.com/coppermine/ stramm.st.funpic.org/ www.lpod.org/coppermine/ www.alanmclark.com/art |

#  Coppermine version: 1.4.19
=begin
<!--
  SVN version info:
  Coppermine version: 1.4.19
  $Revision: 4392 $
  $LastChangedBy: gaugau $
  $Date: 2008-04-16 09:25:35 +0200 (Mi, 16 Apr 2008) $
-->
=end
# <!-- BEGIN album_list -->
# Powered by <a href="http://coppermine-gallery.net/" title="Coppermine Photo Gallery" rel="external">Coppermine Photo Gallery</a>
# <!--Coppermine Photo Gallery 1.4.19 (stable)-->


# with joomla
# <!-- +++++++++++++++++++ coppermine begin +++++++++++++++++++ -->

# <title>Coppermine Photo Gallery - Home</title>
# <!--Coppermine Photo Gallery 1.3.2-->
# Powered by <a href="http://coppermine.sf.net/" target="_blank">Coppermine Photo Gallery</a>

# <!--Coppermine Photo Gallery 1.4.10 (stable)-->
# Powered by <a href="http://coppermine.sourceforge.net/" title="Coppermine Photo Gallery" rel="external">Coppermine Photo Gallery</a>

# <a href="displayimage.php?album=lastup&amp;cat=0&amp;pos=6">

matches [
{:name=>"default title", 
:probability=>100,
:text=>"<title>Coppermine Photo Gallery - Home</title>" },

{:name=>"SVN comments", 
:probability=>100,
:regexp=>/Coppermine version: ([0-9\.]+)[ \n]*\$Revision/ },

{:name=>"BEGIN album list", 
:probability=>50,
:text=>"<!-- BEGIN album_list -->" },

{:name=>"powered by link", 
:probability=>100,
:regexp=>/Powered by <a href="http:\/\/(coppermine-gallery.net|coppermine.sf.net|coppermine.sourceforge.net)\/" [^>]+>Coppermine Photo Gallery<\/a>/ },

{:name=>"version in comments",
:probability=>100,
:regexp=>/<!--Coppermine Photo Gallery ([\d\.]+)[^>]+-->/ },

{:name=>"displayimage.php url",
:probability=>50,
:regexp=>/<a href="[^"]*\/displayimage.php\?album=[a-z0-9]+&pos=">/ } # "
								]
# 

def passive
	m=[]
	
	if @body =~ /<!--Coppermine Photo Gallery ([\d\.]+)[^>]+-->/
		version=@body.scan(/<!--Coppermine Photo Gallery ([\d\.]+[^>]+)-->/)[0][0]
		m << {:probability=>100,:name=>"html comments",:version=>version} 
	end
	
	m
end

end



