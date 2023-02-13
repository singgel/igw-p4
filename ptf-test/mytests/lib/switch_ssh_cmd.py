#!/usr/bin/env python2.7

import os
import paramiko
from lib.logger import Logger

def run_cmd(cmd):
    #logger_ccs = Logger.get_logger(Logger.LOGGER_TYPE_CCS)
    p = os.popen(cmd)
    stdout = p.read()
    #logger_ccs.debug("Run command %s, stdout: %s", cmd, stdout)
    return stdout

class SwitchSshTunnel(object):
    def __init__(self, ip, username='admin', password='YourPass'):
        self.__ip=ip
        self.__username = username
        self.__password = password
        self.__ssh = None
        self.__chan = None
        self.__logger = Logger.get_logger(Logger.LOGGER_TYPE_FRAMEWORK)
    def __def__(self):
        self.__ssh.close()

    def _start_ssh_session(self):
        self.__ssh = paramiko.SSHClient()
        self.__ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        self.__ssh.connect(self.__ip, 22, self.__username, self.__password)

    def run_bash_command(self, command):
        if (self.__ssh is None):
            self._start_ssh_session()
        stdin, stdout, stderr = self.__ssh.exec_command(command)
        return stdout.read(), stderr.read()

if __name__ == "__main__":
    test = SwitchSshTunnel('10.226.137.236', 'root', 'onl')
    stdout, stderr = test.run_bash_command('df -h')
    print stdout
    print "-----------------"
    print stderr
