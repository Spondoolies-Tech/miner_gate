import sys
from zabbix import zapi

try:
    groupname = sys.argv[1]
except Exception:
    print "usage: %s groupname"%sys.argv[0]
    sys.exit()


#group id
host_group_id = zapi.hostgroup.get(filter={"name":groupname})[0]
print "group id: %s" % host_group_id["groupid"]

# get hosts
hosts = [h["hostid"] for h in zapi.host.get(groupids=host_group_id["groupid"], output="hostid")]

"""
not necessary
# for each host, delete items
for h in hosts:
    h.update({"templateid":0})
    zapi.host.update(h)
    #zapi.items.delete(filter=h)
"""
while hosts:
    print "%d hosts to delete" % len(hosts) 
    zapi.host.delete(*hosts[0:10])
    hosts=hosts[10:]


# disconnect any templates from group
templates=[]
for t in zapi.template.get(groupids=host_group_id):
    hosts = zapi.host.get(filter=t)
    if not hosts:
        templates.append(t["templateid"])

if templates:
    print "deleting %d templates" % len(templates)
    zapi.template.delete(*templates)

#if there are anymore connected templates, disconnect them
templates = [t["templateid"] for t in zapi.template.get(groupids=host_group_id)]
if templates:
    print "disconnecting %d templates" % len(templates)
    zapi.hostgroup.massremove(groups=[host_group_id], templateids=templates)

# delete group
zapi.hostgroup.delete(host_group_id["groupid"])
print "deleted group %s [id: %s]" % (groupname, host_group_id["groupid"])
