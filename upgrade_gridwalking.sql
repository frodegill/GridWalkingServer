USE gridwalking;

ALTER TABLE user RENAME TO user_tmp;

CREATE TABLE IF NOT EXISTS `gridwalking`.`user` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `guid` VARCHAR(36) NOT NULL,
  `username` VARCHAR(255) NOT NULL,
  `score` INT UNSIGNED NOT NULL,
  `l13` INT UNSIGNED NOT NULL,
  `l12` INT UNSIGNED NOT NULL,
  `l11` INT UNSIGNED NOT NULL,
  `l10` INT UNSIGNED NOT NULL,
  `l9` INT UNSIGNED NOT NULL,
  `l8` INT UNSIGNED NOT NULL,
  `l7` INT UNSIGNED NOT NULL,
  `l6` INT UNSIGNED NOT NULL,
  `l5` INT UNSIGNED NOT NULL,
  `l4` INT UNSIGNED NOT NULL,
  `l3` INT UNSIGNED NOT NULL,
  `l2` INT UNSIGNED NOT NULL,
  `l1` INT UNSIGNED NOT NULL,
  `l0` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`id`),
  INDEX `score_idx` (`score` ASC),
  UNIQUE INDEX `guid_UNIQUE` (`guid` ASC))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8
COLLATE = utf8_danish_ci;

INSERT INTO user (guid, username, score, l13, l12, l11, l10, l9, l8, l7, l6, l5, l4, l3, l2, l1, l0)
SELECT u.id, u.username, u.score, u.l13, u.l12, u.l11, u.l10, u.l9, u.l8, u.l7, u.l6, u.l5, u.l4, u.l3, u.l2, u.l1, u.l0 
FROM user_tmp u;

DROP TABLE user_tmp;

-- -----------------------------------------------------
-- Table `gridwalking`.`grid`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `gridwalking`.`grid` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `owner` INT UNSIGNED NOT NULL,
  `level` INT UNSIGNED NOT NULL,
  `grid` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`id`),
  INDEX `fk_grid_user1_idx` (`owner` ASC),
  CONSTRAINT `fk_grid_user`
    FOREIGN KEY (`owner`)
    REFERENCES `gridwalking`.`user` (`id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;
