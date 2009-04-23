/*
SQLyog Community Edition- MySQL GUI v8.03 
MySQL - 5.0.51b-community-nt : Database - miss
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

CREATE DATABASE /*!32312 IF NOT EXISTS*/`miss` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `miss`;

/*Table structure for table `buffs` */

DROP TABLE IF EXISTS `buffs`;

CREATE TABLE `buffs` (
  `entry` char(255) NOT NULL default '',
  `type_index` mediumint(11) default '0',
  `eff_index` mediumint(11) default NULL,
  `param1_id` int(11) default NULL,
  `param2_value` int(11) default NULL,
  `param3_pt` int(11) default NULL,
  `param4_mvalue` int(11) default NULL,
  PRIMARY KEY  (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*Data for the table `buffs` */

insert  into `buffs`(`entry`,`type_index`,`eff_index`,`param1_id`,`param2_value`,`param3_pt`,`param4_mvalue`) values ('distance_1',2,0,108,-90,0,2),('distance_2',2,1,79,100,0,1),('distance_3',2,2,9,500,0,0),('freecast_1',7,2,72,-100,0,126),('freecast_2',7,1,107,-10000,0,126),('freecast_3',7,0,108,-90,0,2),('heal_1',4,0,108,-90,0,2),('heal_2',4,1,65,400,0,0),('heal_3',4,2,136,250,0,0),('magicdamage_1',5,1,72,-50,0,126),('magicdamage_2',5,2,79,400,0,1),('magicdamage_3',5,0,108,-90,0,2),('magicrogue_1',6,2,71,100,0,126),('magicrogue_2',6,1,65,400,0,0),('magicrogue_3',6,0,108,-90,0,2),('melee_1',1,0,108,-90,0,2),('melee_2',1,1,79,100,0,1),('melee_3',1,2,9,500,0,0),('rogue_1',3,0,108,-90,0,2),('rogue_2',3,1,52,100,0,0),('rogue_3',3,2,9,400,0,0),('tank_1',0,0,108,400,0,2),('tank_2',0,1,87,-90,0,127),('tank_3',0,2,118,250,0,0);

/*Table structure for table `mercenaries` */

DROP TABLE IF EXISTS `mercenaries`;

CREATE TABLE `mercenaries` (
  `type` char(255) NOT NULL default '',
  `base_stock` mediumint(9) default NULL,
  `price` mediumint(9) default NULL,
  `cooldown` mediumint(9) default NULL,
  `entry_1` mediumint(9) default NULL,
  `entry_2` mediumint(9) default NULL,
  `entry_3` mediumint(9) default NULL,
  `entry_4` mediumint(9) default NULL,
  `entry_5` mediumint(9) default NULL,
  `entry_6` mediumint(9) default NULL,
  `entry_7` mediumint(9) default NULL,
  `entry_8` mediumint(9) default NULL,
  `entry_9` mediumint(9) default NULL,
  `entry_10` mediumint(9) default NULL,
  `summon_entry` mediumint(9) default NULL,
  `summon_nb_min` mediumint(9) default NULL,
  `summon_nb_max` mediumint(9) default NULL,
  `summon_armor_a` float(11,0) default NULL,
  `summon_armor_b` float(11,0) default NULL,
  `summon_minattack_a` float(11,0) default NULL,
  `summon_minattack_b` float(11,0) default NULL,
  `summon_maxattack_a` float(11,0) default NULL,
  `summon_maxattack_b` float(11,0) default NULL,
  `summon_apower_a` float(11,0) default NULL,
  `summon_apower_b` float(11,0) default NULL,
  `summon_interval_min` mediumint(9) default NULL,
  `summon_interval_max` mediumint(9) default NULL,
  `summon_spell_entry` mediumint(9) default NULL,
  `summon_HP_a` mediumint(9) default NULL,
  `summon_HP_b` mediumint(9) default NULL,
  `summon_lifespan` mediumint(9) default NULL,
  `follow_gap` mediumint(9) default NULL,
  `follow_dist_min` mediumint(9) default NULL,
  `follow_dist_max` mediumint(9) default NULL,
  `follow_moy_angle` mediumint(9) default NULL,
  `follow_sd_angle` mediumint(9) default NULL,
  `spell1_interval_min` mediumint(9) default NULL,
  `spell1_interval_max` mediumint(9) default NULL,
  `spell1_effect` int(11) default NULL,
  `spell1_length` mediumint(9) default NULL,
  `spell1_entry` mediumint(9) default NULL,
  `spell2_interval_min` mediumint(9) default NULL,
  `spell2_interval_max` mediumint(9) default NULL,
  `spell2_effect` int(11) default NULL,
  `spell2_length` mediumint(9) default NULL,
  `spell2_entry` mediumint(9) default NULL,
  `spell3_interval_min` mediumint(9) default NULL,
  `spell3_interval_max` mediumint(9) default NULL,
  `spell3_effect` int(11) default NULL,
  `spell3_length` mediumint(9) default NULL,
  `spell3_entry` mediumint(9) default NULL,
  `spell4_interval_min` mediumint(9) default NULL,
  `spell4_interval_max` mediumint(9) default NULL,
  `spell4_effect` int(11) default NULL,
  `spell4_length` mediumint(9) default NULL,
  `spell4_entry` mediumint(9) default NULL,
  `spell5_interval_min` mediumint(9) default NULL,
  `spell5_interval_max` mediumint(9) default NULL,
  `spell5_effect` int(11) default NULL,
  `spell5_length` mediumint(9) default NULL,
  `spell5_entry` mediumint(9) default NULL,
  `is_non_attackable` int(11) default NULL,
  PRIMARY KEY  (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*Data for the table `mercenaries` */

insert  into `mercenaries`(`type`,`base_stock`,`price`,`cooldown`,`entry_1`,`entry_2`,`entry_3`,`entry_4`,`entry_5`,`entry_6`,`entry_7`,`entry_8`,`entry_9`,`entry_10`,`summon_entry`,`summon_nb_min`,`summon_nb_max`,`summon_armor_a`,`summon_armor_b`,`summon_minattack_a`,`summon_minattack_b`,`summon_maxattack_a`,`summon_maxattack_b`,`summon_apower_a`,`summon_apower_b`,`summon_interval_min`,`summon_interval_max`,`summon_spell_entry`,`summon_HP_a`,`summon_HP_b`,`summon_lifespan`,`follow_gap`,`follow_dist_min`,`follow_dist_max`,`follow_moy_angle`,`follow_sd_angle`,`spell1_interval_min`,`spell1_interval_max`,`spell1_effect`,`spell1_length`,`spell1_entry`,`spell2_interval_min`,`spell2_interval_max`,`spell2_effect`,`spell2_length`,`spell2_entry`,`spell3_interval_min`,`spell3_interval_max`,`spell3_effect`,`spell3_length`,`spell3_entry`,`spell4_interval_min`,`spell4_interval_max`,`spell4_effect`,`spell4_length`,`spell4_entry`,`spell5_interval_min`,`spell5_interval_max`,`spell5_effect`,`spell5_length`,`spell5_entry`,`is_non_attackable`) values ('mage',5,1,10000,50570,50571,50572,50573,50574,50575,50576,50577,50578,50579,50534,0,0,0,0,0,0,0,0,0,0,12,12,44263,0,0,30,4,3,6,360,20,0,0,0,0,28272,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),('paladin',5,1,10000,50560,50561,50562,50563,50564,50565,50566,50567,50568,50569,50533,0,0,0,0,15,0,0,0,0,0,0,0,40836,0,0,30,4,3,6,360,20,31,60,10,0,25233,12,24,1,20,34254,60,120,0,60,9800,120,180,0,0,25435,0,0,0,20,1020,0),('shaman',5,1,10000,50540,50541,50542,50543,50544,50545,50546,50547,50548,50549,50531,1,1,240,0,1,0,2,0,5,125,9,12,0,400,400,60,4,3,6,360,20,31,60,0,30,2825,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1),('warlock',5,1,10000,50550,50551,50552,50553,50554,50555,50556,50557,50558,50559,50532,1,3,0,0,10,0,20,0,50,225,60,180,999,1,30,24,4,3,6,360,20,31,40,100,25,41170,31,40,100,25,41170,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);

/*Table structure for table `texts` */

DROP TABLE IF EXISTS `texts`;

CREATE TABLE `texts` (
  `entry` mediumint(9) NOT NULL default '0',
  `lang0` char(255) default NULL,
  `lang1` char(255) default NULL,
  `lang2` char(255) default NULL,
  `lang3` char(255) default NULL,
  `lang4` char(255) default NULL,
  `lang5` char(255) default NULL,
  `lang6` char(255) default NULL,
  `lang7` char(255) default NULL,
  `lang8` char(255) default NULL,
  PRIMARY KEY  (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*Data for the table `texts` */

insert  into `texts`(`entry`,`lang0`,`lang1`,`lang2`,`lang3`,`lang4`,`lang5`,`lang6`,`lang7`,`lang8`) values (1,'No mages available.',NULL,'D\'solé gars, j\'ai aucun mage disponible à l\'heure actuelle.',NULL,NULL,NULL,NULL,NULL,NULL),(2,'Thank you for buying this mage.',NULL,'Tiens, prends donc c\'t extincteur au cas où il mettrait le feu à quelqu\'un...',NULL,NULL,NULL,NULL,NULL,NULL),(3,'No shamans available.',NULL,'Tu tombes mal, j\'ai pas d\'shaman de libre aujourd\'hui...',NULL,NULL,NULL,NULL,NULL,NULL),(4,'Thank you for buying this shaman.',NULL,'Et un shaman qui roule, un!',NULL,NULL,NULL,NULL,NULL,NULL),(5,'No paladins available.',NULL,'Ca tombe vraiment mal, mon dernier paladin s\'est sacrifié pour sauver la vie à un voyageur de passage... revenez plus tard!',NULL,NULL,NULL,NULL,NULL,NULL),(6,'Thank you for buying this paladin.',NULL,'Et une boite de conserve de vendue, une!',NULL,NULL,NULL,NULL,NULL,NULL),(7,'No warlocks available.',NULL,'Oh c\'est bête, mon dernier démoniste est parti promener son marcheur du vide...',NULL,NULL,NULL,NULL,NULL,NULL),(8,'Thank you for buying this warlock.',NULL,'Et abimez pas sa robe, ça vaut une fortune ces machins!',NULL,NULL,NULL,NULL,NULL,NULL),(9,'Buy a mage.',NULL,'Acheter un mage.',NULL,NULL,NULL,NULL,NULL,NULL),(10,'Buy a shaman.',NULL,'Acheter un shaman.',NULL,NULL,NULL,NULL,NULL,NULL),(11,'Buy a paladin.',NULL,'Acheter un paladin.',NULL,NULL,NULL,NULL,NULL,NULL),(12,'Buy a warlock.',NULL,'Acheter un démoniste.',NULL,NULL,NULL,NULL,NULL,NULL),(13,'Mercenaries summoning service.',NULL,'Service de vente de mercenaires.',NULL,NULL,NULL,NULL,NULL,NULL),(14,'You must be in a goup to buy one of my mercenaries.',NULL,'Vous devez être groupé pour pouvoir acheter un de mes mercenaires.',NULL,NULL,NULL,NULL,NULL,NULL),(15,'You will pay',NULL,'Prix :',NULL,NULL,NULL,NULL,NULL,NULL),(16,'Sorry, I am not allowed to do this.',NULL,'Désolé, je ne peux pas faire ça.',NULL,NULL,NULL,NULL,NULL,NULL),(17,'I will refuse any level 0 commands from now.',NULL,'Je n\'obéirai plus aux commandes de niveau 0.',NULL,NULL,NULL,NULL,NULL,NULL),(18,'I will accept all commands.',NULL,'J\'obéirai à toutes les commandes.',NULL,NULL,NULL,NULL,NULL,NULL),(19,'All cooldowns set to',NULL,'Cooldowns réinitialisés à',NULL,NULL,NULL,NULL,NULL,NULL),(20,'All my summons have been expelled.',NULL,'Toutes mes invocations ont été renvoyées.',NULL,NULL,NULL,NULL,NULL,NULL),(21,'I must be unsummoned.',NULL,'Je dois être désinvoqué.',NULL,NULL,NULL,NULL,NULL,NULL),(22,'Names stack cleared.',NULL,'Names stack cleared.',NULL,NULL,NULL,NULL,NULL,NULL),(23,'Headers stack cleared.',NULL,'Headers stack cleared.',NULL,NULL,NULL,NULL,NULL,NULL),(24,'Summons stack cleared.',NULL,'Summons stack cleared.',NULL,NULL,NULL,NULL,NULL,NULL),(25,'Spells stack cleared.',NULL,'Spells stack cleared.',NULL,NULL,NULL,NULL,NULL,NULL),(26,'I can only clear following stacks :',NULL,'I can only clear following stacks :',NULL,NULL,NULL,NULL,NULL,NULL),(27,'Freeze.',NULL,'Freeze.',NULL,NULL,NULL,NULL,NULL,NULL),(28,'Volatile consts reloaded.',NULL,'Volatile consts reloaded.',NULL,NULL,NULL,NULL,NULL,NULL),(29,'Group reset.',NULL,'Groupe réintialisé.',NULL,NULL,NULL,NULL,NULL,NULL),(30,'You must be in a group to reset group.',NULL,'Vous devez être groupé pour réinitialiser le groupe.',NULL,NULL,NULL,NULL,NULL,NULL),(31,'But, what would I attack him, sir?',NULL,'Mais, pourquoi est-ce que je l\'attaquerais, maître?',NULL,NULL,NULL,NULL,NULL,NULL),(32,'Attack!',NULL,'A l\'attaque!',NULL,NULL,NULL,NULL,NULL,NULL),(33,'Okay sir, I\'m not attacking anyone.',NULL,'Bien maître, je n\'attaquerai pas.',NULL,NULL,NULL,NULL,NULL,NULL),(34,'Fine sir, I\'ll keep my position.',NULL,'Bien maître, je tiens ma position.',NULL,NULL,NULL,NULL,NULL,NULL),(35,'I got your back, sir!',NULL,'Je vous suis, maître.',NULL,NULL,NULL,NULL,NULL,NULL),(36,'I won\'t sumon any creature, sir.',NULL,'Je n\'invoquerai aucune créature, maître.',NULL,NULL,NULL,NULL,NULL,NULL),(37,'Okay sir, I may summon creatures now.',NULL,'Bien maître, je pourrai invoquer des créatures maintenant.',NULL,NULL,NULL,NULL,NULL,NULL),(38,'I won\'t cast any spell, sir.',NULL,'Je ne lancerai aucun sort, maître.',NULL,NULL,NULL,NULL,NULL,NULL),(39,'Okay sir, I may cast spells now.',NULL,'Bien maître, je pourrai lancer des sorts maitenant.',NULL,NULL,NULL,NULL,NULL,NULL),(40,'Farewell, sir.',NULL,'Adieu, maître.',NULL,NULL,NULL,NULL,NULL,NULL),(41,'Ok sir, I will talk less.',NULL,'Bien maître, je parlerai moins.',NULL,NULL,NULL,NULL,NULL,NULL),(42,'Ok sir, I will talk more.',NULL,'Bien maître, je parlerai plus.',NULL,NULL,NULL,NULL,NULL,NULL),(43,'I can react to :',NULL,'Je peux obéir aux commandes :',NULL,NULL,NULL,NULL,NULL,NULL),(44,'Current health :',NULL,'Santé :',NULL,NULL,NULL,NULL,NULL,NULL),(45,'Current mana :',NULL,'Mana :',NULL,NULL,NULL,NULL,NULL,NULL),(46,'I can only report :',NULL,'Je ne peux afficher que :',NULL,NULL,NULL,NULL,NULL,NULL),(47,'Melee bloodlust!',NULL,'Melee bloodlust!',NULL,NULL,NULL,NULL,NULL,NULL),(48,'Distance bloodlust!',NULL,'Distance bloodlust!',NULL,NULL,NULL,NULL,NULL,NULL),(49,'Rogue bloodlust!',NULL,'Rogue bloodlust!',NULL,NULL,NULL,NULL,NULL,NULL),(50,'Tank bloodlust!',NULL,'Tank bloodlust!',NULL,NULL,NULL,NULL,NULL,NULL),(51,'Heal bloodlust!',NULL,'Heal bloodlust!',NULL,NULL,NULL,NULL,NULL,NULL),(52,'Magic damage bloodlust!',NULL,'Magic damage bloodlust!',NULL,NULL,NULL,NULL,NULL,NULL),(53,'Magic rogue bloodlust!',NULL,'Magic rogue bloodlust!',NULL,NULL,NULL,NULL,NULL,NULL),(54,'Freecast bloodlust!',NULL,'Freecast bloodlust!',NULL,NULL,NULL,NULL,NULL,NULL),(55,'Come to me, my beast!',NULL,'Venez à moi, forces de la nature! Esprit du loup!',NULL,NULL,NULL,NULL,NULL,NULL),(56,'I feel them... they... are... coming...',NULL,'Je les sens... ils... arrivent...',NULL,NULL,NULL,NULL,NULL,NULL),(57,'I only need a few seconds to summon them now...',NULL,'Ils seront bientôt là maintenant...',NULL,NULL,NULL,NULL,NULL,NULL),(58,'My spawnlings are on their way to our world...',NULL,'Mes mignons sont en chemin!',NULL,NULL,NULL,NULL,NULL,NULL),(59,'I\'m going to start channeling my summoning!',NULL,'L\'invocation peut commencer...',NULL,NULL,NULL,NULL,NULL,NULL),(60,'I am exhausted... I must recover my power...',NULL,'Je suis épuisé... je dois... reprendre mes forces...',NULL,NULL,NULL,NULL,NULL,NULL),(61,'Physical curse!',NULL,'Malédiction physique!',NULL,NULL,NULL,NULL,NULL,NULL),(62,'Magical curse!',NULL,'Malédiction magique!',NULL,NULL,NULL,NULL,NULL,NULL),(63,'Come to me, my minions!',NULL,'Venez à moi, ambassadeurs du néant!',NULL,NULL,NULL,NULL,NULL,NULL),(64,'Holy Light, come to me! Greater Heal!',NULL,'Lumière sacrée!',NULL,NULL,NULL,NULL,NULL,NULL),(65,'Holy Light shalt support thy strength! Rejuvenation!',NULL,'La lumière sacrée décuplera vos forces! Regénération!',NULL,NULL,NULL,NULL,NULL,NULL),(66,'Light will grant thee its protection! Holy Shield!',NULL,'La lumière vous confèrera sa protection! Bouclier sacré!',NULL,NULL,NULL,NULL,NULL,NULL),(67,'Surrounded by the Light, thou shalt fear nothing! Divine Shield!',NULL,'Entouré par la lumière, plus rien ne pourra s\'opposer à vous! Bénédiction de protection!',NULL,NULL,NULL,NULL,NULL,NULL),(68,'I call upon thee in the land of the dead, bring back this soul from Purgatory! Resurrection!',NULL,'Résurrection!',NULL,NULL,NULL,NULL,NULL,NULL),(69,'Sheeps stack cleared.',NULL,'Sheeps stack cleared.',NULL,NULL,NULL,NULL,NULL,NULL),(70,'You be under control of me! Polymorphation!',NULL,'Tu être sous mon contrôle! Polymorphation!',NULL,NULL,NULL,NULL,NULL,NULL),(71,'The arcanes shall burn you! Energy bolt!',NULL,'Les arcanes vous consummeront! Boule d\'énergie!',NULL,NULL,NULL,NULL,NULL,NULL);

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
