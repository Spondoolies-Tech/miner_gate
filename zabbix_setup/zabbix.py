from pyzabbix import ZabbixAPI
import sys
server = "http://127.0.0.1/zabbix"
user = "Admin"
pw = "zabbix"

zapi_interfaces=[
            {
                "type": 1,
                "main": 1,
                "useip": 1,
                "ip": "127.0.0.1",
                "dns": "",
                "port": "10050"
            }
        ]

zapi = ZabbixAPI(server)

# Login to the Zabbix API
zapi.login(user, pw)

