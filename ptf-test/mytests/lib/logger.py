#!/usr/bin/env python2.7

import logging

class Logger(object):
    LOGGER_TYPE_CCS = "ccs"
    LOGGER_TYPE_PARSER ="parser"
    LOGGER_TYPE_DATAPLANE = "dataplane"
    LOGGER_TYPE_FRAMEWORK = "framework"
    LOGGER_TYPE_TESTCASE = 'testcase'

    LOGLEVEL_INFO = logging.INFO
    LOGLEVEL_DEBUG = logging.DEBUG
    LOGLEVEL_ERR = logging.ERROR
    LOGLEVEL_CRITICAL = logging.CRITICAL
    LOGLEVEL_FATAL = logging.FATAL

    LOGLEVEL_DEFAULT = LOGLEVEL_INFO

    logger_dict = {}
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    fh = logging.StreamHandler()
    fh.setFormatter(formatter)

    @classmethod
    def get_logger(cls, logger_type="default"):
        logger_type = logger_type.lower()
        if logger_type not  in Logger.logger_dict:
            logger = logging.getLogger(logger_type)
            logger.addHandler(cls.fh)
            logger.setLevel(cls.LOGLEVEL_DEFAULT)
            cls.logger_dict[logger_type] = logger
        return cls.logger_dict[logger_type]


    @classmethod
    def set_log_level(cls, level, logger_type=None):
        if logger_type is None:
            for iterator in Logger.logger_dict:
                cls.logger_dict[iterator].setLevel(level)
        else:
            if logger_type in logger_dict:
                cls.logger_dict[logger_type].setLevel(level)


