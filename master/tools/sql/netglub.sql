SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

CREATE TABLE IF NOT EXISTS `entities` (
  `name` varchar(255) NOT NULL,
  `browsable` tinyint(1) NOT NULL default '1',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

INSERT INTO `entities` (`name`, `browsable`) VALUES
('autonomous-system', 1),
('dns-name', 1),
('document', 1),
('domain-name', 1),
('email-address', 1),
('ip-address', 1),
('ip-block', 1),
('location', 1),
('mx-record', 1),
('ns-record', 1),
('operating-system', 1),
('person', 1),
('phone-number', 1),
('phrase', 1),
('port', 1),
('url', 1),
('website', 1),
('unknown', 1);

CREATE TABLE IF NOT EXISTS `entities_category` (
  `name` varchar(255) NOT NULL,
  `shortname` varchar(255) NOT NULL,
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Contenu de la table `entities_category`
--

INSERT INTO `entities_category` (`name`, `shortname`) VALUES
('infrastructure', 'Infrastructure'),
('pentesting', 'Pen Testing'),
('personnal', 'Personnal');

-- --------------------------------------------------------

--
-- Structure de la table `groups`
--

CREATE TABLE IF NOT EXISTS `groups` (
  `groupname` varchar(255) NOT NULL,
  `description` varchar(255) NOT NULL,
  PRIMARY KEY  (`groupname`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Contenu de la table `groups`
--

INSERT INTO `groups` (`groupname`, `description`) VALUES
('community', 'Community group number 1 for demonstration'),
('commercial_level1', 'Commercial group level 1'),
('testGroup', 'without users');

-- --------------------------------------------------------

--
-- Structure de la table `join_entities_category_entities`
--

CREATE TABLE IF NOT EXISTS `join_entities_category_entities` (
  `category_name` varchar(255) NOT NULL,
  `entity_name` varchar(255) NOT NULL,
  PRIMARY KEY  (`category_name`,`entity_name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Contenu de la table `join_entities_category_entities`
--

INSERT INTO `join_entities_category_entities` (`category_name`, `entity_name`) VALUES
('infrastructure', 'autonomous-system'),
('infrastructure', 'dns-name'),
('infrastructure', 'domain-name'),
('infrastructure', 'ip-address'),
('infrastructure', 'ip-block'),
('infrastructure', 'mx-record'),
('infrastructure', 'ns-record'),
('infrastructure', 'unknown'),
('infrastructure', 'url'),
('infrastructure', 'website'),
('pentesting', 'operating-system'),
('pentesting', 'port'),
('personnal', 'document'),
('personnal', 'email-address'),
('personnal', 'location'),
('personnal', 'person'),
('personnal', 'phone-number'),
('personnal', 'phrase');

-- --------------------------------------------------------

--
-- Structure de la table `join_groups_transforms`
--

CREATE TABLE IF NOT EXISTS `join_groups_transforms` (
  `groupname` varchar(255) NOT NULL default 'community',
  `transform_name` varchar(255) NOT NULL,
  PRIMARY KEY  (`transform_name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Contenu de la table `join_groups_transforms`
--

INSERT INTO `join_groups_transforms` (`groupname`, `transform_name`) VALUES
('community', 'DDToIp'),
('community', 'DnsNameToWebsite'),
('community', 'DnsWebsiteToDomain'),
('community', 'DomainEmailToEmailSE'),
('community', 'DomainIpToEmailWhois'),
('community', 'DomainToDnsNameBrute'),
('community', 'DomainToDnsNameZN'),
('community', 'DomainToDomainExpand'),
('community', 'DomainToDomainTLD'),
('community', 'DomainToEmailPGP'),
('community', 'DomainToEmailSEDomOnly'),
('community', 'DomainToLocationWhois'),
('community', 'DomainToPersonPGP'),
('community', 'DomainToWebsite'),
('community', 'EmailIpToWebsiteSE'),
('community', 'EmailToDomain'),
('community', 'EmailToEmailSamePGP'),
('community', 'EmailToPersonPGP'),
('community', 'EmailWebsiteToUrlSE'),
('commercial_level1', 'IpBlockToPorts'),
('commercial_level1', 'IpToDomainSharedIp'),
('community', 'IpToIpBlockCuts'),
('community', 'IpToLocation'),
('commercial_level1', 'IpToOs'),
('commercial_level1', 'IpToOsNmap'),
('commercial_level1', 'IpToOsPhp'),
('commercial_level1', 'IpToPorts'),
('community', 'PersonToEmailPGP'),
('community', 'PersonToEmailSE'),
('community', 'PersonToPersonPGP'),
('community', 'PersonToWebsiteSE'),
('community', 'UrlToEmail'),
('community', 'UrlToUrl'),
('community', 'UrlToUrlInk'),
('community', 'UrlToWebsite'),
('community', 'WebsiteToEmailMirror'),
('community', 'WebsiteToIp'),
('community', 'WebsiteToWebsiteInk'),
('community', 'WebsiteToWebsiteMirror'),
('community', 'PhraseToUrlSE'),
('community', 'UrlToEntitiesNER'),
('community', 'IpBlockToIpAddress'),
('community', 'DummyTransform'),
('community', 'DomainToDomainTopLevel'),
('community', 'DomainToMX'),
('community', 'DomainToNS'),
('community', 'WebsiteToUrlMirror'),
('community', 'IpToDnsName'),
('community', 'IpBlockToDnsName'),
('community', 'DomainToDnsNameSE'),
('community', 'WebsiteToEntitiesNER'),
('community', 'WebsiteToPhraseWhatWeb'),
('community', 'IpToAsWhois'),
('commercial_level1', 'IpToIpBlock'),
('community', 'IpToWiki');

-- --------------------------------------------------------

--
-- Structure de la table `join_groups_users`
--

CREATE TABLE IF NOT EXISTS `join_groups_users` (
  `groupname` varchar(255) NOT NULL,
  `username` varchar(255) NOT NULL,
  PRIMARY KEY  (`groupname`,`username`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Contenu de la table `join_groups_users`
--

INSERT INTO `join_groups_users` (`groupname`, `username`) VALUES
('commercial_level1', 'commercial1'),
('community', 'commercial1'),
('community', 'community1');

-- --------------------------------------------------------

--
-- Structure de la table `join_transforms_category_transforms`
--

CREATE TABLE IF NOT EXISTS `join_transforms_category_transforms` (
  `category_name` varchar(255) NOT NULL,
  `transform_name` varchar(255) NOT NULL,
  PRIMARY KEY  (`category_name`,`transform_name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Contenu de la table `join_transforms_category_transforms`
--

INSERT INTO `join_transforms_category_transforms` (`category_name`, `transform_name`) VALUES
('default', 'domainIpToEmailWhois'),
('default', 'DomainToDomainTLD'),
('default', 'DomainToEmailPGP'),
('default', 'DomainToLocationWhois'),
('default', 'DomainToPersonPGP'),
('default', 'DummyTransform'),
('default', 'EmailToEmailSamePGP'),
('default', 'EmailToPersonPGP'),
('default', 'IpToDomainSharedIp'),
('default', 'IpToIpBlockCuts'),
('default', 'IpToLocation'),
('default', 'IpToWiki'),
('default', 'PersonToEmailPGP'),
('default', 'PersonToPersonPGP'),
('default', 'WebsiteToEntitiesNER'),
('default', 'WebsiteToUrlMirror'),
('dns_lookup', 'DDToIp'),
('dns_lookup', 'DomainToDnsNameBrute'),
('dns_lookup', 'DomainToDnsNameSE'),
('dns_lookup', 'DomainToDnsNameZN'),
('dns_lookup', 'DomainToMX'),
('dns_lookup', 'DomainToNS'),
('dns_lookup', 'IpBlockToDnsName'),
('dns_lookup', 'IpToAsWhois'),
('dns_lookup', 'IpToDnsName'),
('dns_lookup', 'WebsiteToIp'),
('fingerprint', 'IpBlockToIpAddress'),
('fingerprint', 'IpBlockToPorts'),
('fingerprint', 'IpToIpBlock'),
('fingerprint', 'ipToOs'),
('fingerprint', 'IpToOsNmap'),
('fingerprint', 'ipToOsPhp'),
('fingerprint', 'IpToPorts'),
('fingerprint', 'ipToPorts2'),
('fingerprint', 'WebsiteToPhraseWhatWeb'),
('parse', 'DnsNameToWebsite'),
('parse', 'DnsWebsiteToDomain'),
('parse', 'DomainToDomainTopLevel'),
('parse', 'DomainToWebsite'),
('parse', 'EmailToDomain'),
('parse', 'UrlToEntitiesNER'),
('parse', 'UrlToUrl'),
('parse', 'UrlToWebsite'),
('parse', 'WebsiteToEmailMirror'),
('parse', 'WebsiteToWebsiteMirror'),
('search_engine', 'DomainEmailToEmailSE'),
('search_engine', 'DomainToDomainExpand'),
('search_engine', 'DomainToEmailSEDomOnly'),
('search_engine', 'EmailIpToWebsiteSE'),
('search_engine', 'EmailWebsiteToUrlSE'),
('search_engine', 'PersonToEmailSE'),
('search_engine', 'PersonToWebsiteSE'),
('search_engine', 'PhraseToUrlSE'),
('search_engine', 'UrlToEmail'),
('search_engine', 'UrlToUrlInk'),
('search_engine', 'WebsiteToWebsiteInk');

-- --------------------------------------------------------

--
-- Structure de la table `sessions`
--

CREATE TABLE IF NOT EXISTS `sessions` (
  `session_id` varchar(255) NOT NULL,
  `username` varchar(255) NOT NULL,
  `registration_datetime` datetime NOT NULL,
  `last_modification_datetime` datetime NOT NULL,
  PRIMARY KEY  (`session_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Contenu de la table `sessions`
--


-- --------------------------------------------------------

--
-- Structure de la table `transforms`
--

CREATE TABLE IF NOT EXISTS `transforms` (
  `name` varchar(255) NOT NULL,
  `enable` tinyint(1) NOT NULL default '1',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Contenu de la table `transforms`
--

INSERT INTO `transforms` (`name`, `enable`) VALUES
('DDToIp', 1),
('DnsNameToWebsite', 1),
('DnsWebsiteToDomain', 1),
('DomainEmailToEmailSE', 0),
('DomainIpToEmailWhois', 1),
('DomainToDnsNameBrute', 1),
('DomainToDnsNameZN', 1),
('DomainToDomainExpand', 0),
('DomainToDomainTLD', 1),
('DomainToEmailPGP', 0),
('DomainToEmailSEDomOnly', 0),
('DomainToLocationWhois', 1),
('DomainToPersonPGP', 0),
('DomainToWebsite', 1),
('EmailIpToWebsiteSE', 0),
('EmailToDomain', 1),
('EmailToEmailSamePGP', 0),
('EmailToPersonPGP', 0),
('EmailWebsiteToUrlSE', 0),
('IpBlockToPorts', 0),
('IpToDomainSharedIp', 1),
('IpToIpBlockCuts', 1),
('IpToLocation', 1),
('IpToOs', 0),
('IpToOsNmap', 1),
('IpToOsPhp', 0),
('IpToPorts', 1),
('PhraseToUrlSE', 1),
('PersonToEmailPGP', 0),
('PersonToEmailSE', 1),
('PersonToPersonPGP', 0),
('PersonToWebsiteSE', 0),
('UrlToEmail', 1),
('UrlToUrl', 1),
('UrlToUrlInk', 1),
('UrlToWebsite', 1),
('WebsiteToEmailMirror', 1),
('WebsiteToIp', 1),
('WebsiteToWebsiteInk', 1),
('WebsiteToWebsiteMirror', 1),
('UrlToEntitiesNER', 1),
('IpBlockToIpAddress', 1),
('DummyTransform', 1),
('DomainToDomainTopLevel', 1),
('DomainToMX', 1),
('DomainToNS', 1),
('WebsiteToUrlMirror', 1),
('IpToDnsName', 1),
('IpBlockToDnsName', 1),
('DomainToDnsNameSE', 1),
('WebsiteToEntitiesNER', 1),
('WebsiteToPhraseWhatWeb', 1),
('IpToAsWhois', 1),
('IpToIpBlock', 1),
('IpToWiki', 1);

-- --------------------------------------------------------

--
-- Structure de la table `transforms_category`
--

CREATE TABLE IF NOT EXISTS `transforms_category` (
  `name` varchar(255) NOT NULL,
  `shortname` varchar(255) NOT NULL,
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Contenu de la table `transforms_category`
--

INSERT INTO `transforms_category` (`name`, `shortname`) VALUES
('default', 'Default'),
('fingerprint', 'Fingerprint'),
('dns_lookup', 'DNS Lookup'),
('parse', 'Syntactic Analysis'),
('search_engine', 'Search Engine');

-- --------------------------------------------------------

--
-- Structure de la table `transforms_limit`
--

CREATE TABLE IF NOT EXISTS `transforms_limit` (
  `serial` varchar(255) NOT NULL,
  `ip_address` varchar(255) NOT NULL,
  `date` date NOT NULL,
  `transforms_number` int(11) NOT NULL default '0',
  PRIMARY KEY  (`serial`,`ip_address`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;


CREATE TABLE IF NOT EXISTS `users` (
  `username` varchar(255) NOT NULL,
  `auth_level` enum('NONE','USER','ADMIN') NOT NULL default 'NONE',
  `enable` tinyint(1) NOT NULL default '1',
  `password` varchar(32) NOT NULL,
  `serial` varchar(255) NOT NULL,
  `next_graph_id` int(11) NOT NULL,
  `firstname` varchar(255) NOT NULL,
  `lastname` varchar(255) NOT NULL,
  `email` varchar(255) NOT NULL,
  `registration_datetime` datetime NOT NULL,
  `last_modification_datetime` datetime NOT NULL,
  PRIMARY KEY  (`username`),
  UNIQUE KEY `email` (`email`),
  UNIQUE KEY `serial` (`serial`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Contenu de la table `users`
--

INSERT INTO `users` (`username`, `auth_level`, `enable`, `password`, `serial`, `next_graph_id`, `firstname`, `lastname`, `email`, `registration_datetime`, `last_modification_datetime`) VALUES
('community1', 'USER', 1, '681e97d75e65fe1c14b257c88e5cc01b', '1111-4567-89ab-cdef', 1, 'community1', 'COMMUNITY', 'community1@netglub.org', '2010-04-13 17:07:06', '2010-04-13 17:07:06'),
('commercial1', 'USER', 1, '552a42b69b329bf60d96e1cb6aa5e321', '2222-4567-89ab-cdef', 1, 'commercial1', 'COMMERCIAL', 'commercial1@netglub.org', '2010-04-13 17:07:06', '2010-04-13 17:07:06');
