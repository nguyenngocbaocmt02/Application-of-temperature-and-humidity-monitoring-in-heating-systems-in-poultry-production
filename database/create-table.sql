CREATE DATABASE  IF NOT EXISTS `iot` /*!40100 DEFAULT CHARACTER SET utf8 */ /*!80016 DEFAULT ENCRYPTION='N' */;
USE `iot`;

DROP TABLE IF EXISTS `log`;
CREATE TABLE `light` (
  `time` TIMESTAMP NOT NULL,
  `temp` float NOT NULL,
  `hum` float NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;
