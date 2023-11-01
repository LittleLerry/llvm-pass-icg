import networkx as nx
import pydot
import sys
import re
import argparse

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument('-ig', '--input_graph', help='Input graph file name', required=True)
    parser.add_argument('-og', '--output_graph', help='Output graph file name', required=True)
    args = parser.parse_args()
    return args.input_graph, args.output_graph

def main():
    icg_string = "__ICG_STDOUT__"
    pattern = r'__ICG_STDOUT__{(.+?)}{(.+?)}'
    ig, og = parse_arguments()
    
    G = nx.DiGraph(nx.drawing.nx_pydot.read_dot(ig))
    
    label_node = {}
    
    for attrs in G.nodes(data=True):
        if "label" in attrs[1]:
            key = eval(attrs[1]['label'])
            val = attrs[0]
            label_node[key] = val;
    
    
    for line in sys.stdin:
        line = line.strip()
        match = re.search(pattern, line)
        if match:
            caller = "{"+ match.group(1) +"}"
            callee = "{"+ match.group(2) +"}"
            if caller == callee:
                print("WARN/Give up processing if caller equals to callee: " + line)
            else:
                if (caller,callee in label_node.keys()):
                    node_caller = label_node[caller]
                    node_callee = label_node[callee]
                    G.add_edge(node_caller,node_callee)
                else:
                    print("WARN/Cannot find caller or callee: " + line)
                
        else:
            print("WARN/Bad __ICG_STDOUT__ for line: " + line)
    
    #write_G_to_new_dot
    nx.nx_pydot.write_dot(G,og)
    
            
if __name__ == "__main__":
    main()