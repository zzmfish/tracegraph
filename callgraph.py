#!/usr/bin/python
#encoding=utf-8
import re
import os
import sys

addr2line = sys.argv[1]
exefile = sys.argv[2]
inputfile = sys.argv[3]
outputfile = sys.argv[4]

addr2name = {}
edges = set()
leafs = set()

def get_node_index(name):
  if nodes.count(name) == 0:
    nodes.append(name)
  return nodes.index(name)

f = open(inputfile, 'r')
addrs = []
while True:
  line = f.readline().strip('\r\n')
  if not line: break
  caller, callee, leaf = line.split()
  addrs.append(caller)
  addrs.append(callee)
  edges.add((caller, callee))
  if leaf == '1':
    leafs.add(callee)
f.close()

namespace_pattern = re.compile('^[\w_]+::[\w_]+::')
template_pattern = re.compile('<[^<>]+>')
i = 0
isLineNumber = True
for line in os.popen('%s -f -C -e %s %s' % (addr2line, exefile, ' '.join(addrs))).readlines():
  isLineNumber = not isLineNumber
  if isLineNumber:
    continue
  name = line.strip('\r\n')

  #去掉模板
  while True:
    matchobj = template_pattern.search(name)
    if not matchobj:
      break
    name = name.replace(matchobj.group(0), '')

  #去掉函数参数
  if name[-1] == ')' or (len(name) > 7 and name[-7:] == ') const'):
    try:
      name = name[:name.index('(')]
    except:pass

  #去掉名字空间
  if namespace_pattern.match(name):
    name = name[name.index('::') + 2 :]

  addr2name[addrs[i]] = name
  i += 1
nodes = set(addr2name.values())

f = open('callgraph.dot', 'w')
f.write('digraph G {\n');
nodeIds = {}
i=0
leafNodes = set()

#write leaf nodes
f.write('node [ shape="box" fontsize="10" style="filled" fillcolor="#88dd88" ];\n');
for leafAddr in leafs:
  node = addr2name[leafAddr]
  leafNodes.add(node)
  nodeId = 'node' + str(i)
  f.write('%s [ label="%s" ];\n' % (nodeId, node))
  nodeIds[node] = nodeId
  i += 1
  
#write normal nodes
f.write('node [ style="solid" ];\n');
for node in nodes:
  if node in leafNodes:
    continue
  nodeId = 'node' + str(i)
  f.write('%s [ label="%s" ];\n' % (nodeId, node))
  nodeIds[node] = nodeId
  i += 1

output_edges = []
for edge in edges:
  caller_id = nodeIds[addr2name[edge[0]]]
  callee_id = nodeIds[addr2name[edge[1]]]
  if output_edges.count((caller_id, callee_id)) == 0:
    f.write('%s -> %s\n' % (caller_id, callee_id))
    output_edges.append((caller_id, callee_id))
f.write('}\n');
f.close()

os.system('dot -Tpng callgraph.dot -o ' + outputfile)
