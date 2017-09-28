import re, sys
from collections import defaultdict
import matplotlib.pyplot as plt
import math
import numpy as np
import scipy.stats

# Delay and delivery statistics will be calculated considering each retransmission
# as a new transmission
STAT_CALC = "Individual"
# Delay and delivery statistics will be calculated considering each retransmission
# as part of the initial transmission
# STAT_CALC = "E2E"

# STAT_DESV = "STD_DEV"
STAT_DESV = "CONF_INTERVAL"
STAT_CONFIDENCE = 0.95

#Function to add ticks
def addticks(ax,newLocs,newLabels,pos='x'):
    # Draw to get ticks
    plt.draw()

    # Get existing ticks
    if pos=='x':
        locs = ax.get_xticks().tolist()
        labels=[x.get_text() for x in ax.get_xticklabels()]
    elif pos =='y':
        locs = ax.get_yticks().tolist()
        labels=[x.get_text() for x in ax.get_yticklabels()]
    else:
        print("WRONG pos. Use 'x' or 'y'")
        return

    # Build dictionary of ticks
    Dticks=dict(zip(locs,labels))

    # Add/Replace new ticks
    for Loc,Lab in zip(newLocs,newLabels):
        Dticks[Loc]=Lab

    print Dticks
    # Get back tick lists
    locs=list(Dticks.keys())
    labels=list(Dticks.values())

    # Generate new ticks
    if pos=='x':
        ax.set_xticks(locs)
        ax.set_xticklabels(labels)
    elif pos =='y':
        ax.set_yticks(locs)
        ax.set_yticklabels(labels)

def parse_file(filename):
    print "Parsing", filename

    packetTypes = {
    0: 'SDN_PACKET_CONTROL_FLOW_SETUP',
    1: 'SDN_PACKET_DATA_FLOW_SETUP',
    2: 'SDN_PACKET_CONTROL_FLOW_REQUEST',
    3: 'SDN_PACKET_DATA_FLOW_REQUEST',
    4: 'SDN_PACKET_NEIGHBOR_REPORT',
    5: 'SDN_PACKET_DATA',
    6 + 0xE0 : 'SDN_PACKET_SRC_ROUTED_CONTROL_FLOW_SETUP',
    7 + 0xE0: 'SDN_PACKET_SRC_ROUTED_DATA_FLOW_SETUP',
    8 + 0xD0: 'SDN_PACKET_MULTIPLE_CONTROL_FLOW_SETUP',
    9 + 0xD0: 'SDN_PACKET_MULTIPLE_DATA_FLOW_SETUP',
    10: 'SDN_PACKET_ND',
    11: 'SDN_PACKET_CD',
    12: 'SDN_PACKET_ACK',
    12: 'SDN_PACKET_ACK_BY_FLOW_ID',
    13: 'SDN_PACKET_SRC_ROUTED_ACK',
    14: 'SDN_PACKET_REGISTER_FLOWID',
    15: 'SDN_PACKET_ENERGY_REPORT',
    16: 'SDN_PACKET_ACK_BY_FLOW_ADDRESS'
    }

    tx = defaultdict(list)
    retx = {}
    rx = defaultdict(list)
    txb = defaultdict(list)
    txb = {}
    energy = {}
    fg_time = -1
    # txa = defaultdict(list)

    expression = re.compile(r"""
        (?P<TIME>.*) # First field is time, we do not match any pattern
        (?:\s.*ID:|:) # ID field is different in Cooja GUI and command line (ID: or just a :)
        (?P<ID>\d+) # the actual id is a sequence of numbers
        (?:\s|:).* # finishing id pattern (colon or space) and matching everything else until getting what we want
        =\[\ (?P<ADDR>[0-9A-F][0-9A-F]\ [0-9A-F][0-9A-F])\ \] # address field
        =(?P<RTX>RX|TX(?:A|B|)) # RX or TX
        =(?P<TYPE>[0-9A-F][0-9A-F]) # messageType
        =(?P<SEQNUM>[0-9A-F][0-9A-F]) # sequence number
        =(?P<DEST>.*)= # destination
        """, re.X) #re.X: verbose so we can comment along

    fg_expression = re.compile(r"""
        (?P<TIME>.*) # First field is time, we do not match any pattern
        (?:\s.*ID:|:) # ID field is different in Cooja GUI and command line (ID: or just a :)
        (?P<ID>\d+) # the actual id is a sequence of numbers
        (?:\s|:).* # finishing id pattern (colon or space) and matching everything else until getting what we want
        =(?P<FG>FG) # FG flag
        =(?P<NN>[0-9]*) # number of nodes
        """, re.X) #re.X: verbose so we can comment along

    energy_expression = re.compile(r"""
        (?P<TIME>.*) # First field is time, we do not match any pattern
        (?:\s.*ID:|:) # ID field is different in Cooja GUI and command line (ID: or just a :)
        (?P<ID>\d+) # the actual id is a sequence of numbers
        (?:\s|:).* # finishing id pattern (colon or space) and matching everything else until getting what we want
        (?P<ETAG>E:) # Energy tag
        \s(?P<ENERGY>\d+)\smJ # energy amount
        """, re.X) #re.X: verbose so we can comment along

    # time expression (miliseconds or H+:MM:SS:miliseconds)
    timeExpr = re.compile("(?:(\d+)?:?(\d\d):(\d\d).)?(\d+)")

    # times_test = ("01:27.294", "01:11:37.434", "99999")
    #
    # for t in times_test:
    #     s = timeExpr.match(t)
    #     print t, ":", s.groups() if s!=None else "no match"

    # f = ['5551598:2:~[~=[ 01 00 ]=TX=E7=01==']
    try:
        f = file(filename)
    except:
        print "error on f = file(filename)"
        return

    for l in f:
        # print l.strip()
        s = expression.search(l)
        if s != None:
            # print s.groups()
            time, printid, srcaddr, rxtx, messageType, seqNum, dest = s.groups()

            # print time
            # print printid
            # print srcaddr
            # print rxtx
            # print messageType,
            # print seqNum
            # print dest

            timeMatch = timeExpr.match(time)
            # print timeMatch
            timeMatch = map(lambda(x):float(x) if x != None else 0, timeMatch.groups())
            # print timeMatch
            time = 0
            time += timeMatch[0] * 1000 * 60 * 60 if timeMatch[0] != None else 0
            time += timeMatch[1] * 1000 * 60 if timeMatch[1] != None else 0
            time += timeMatch[2] * 1000 if timeMatch[2] != None else 0
            time += timeMatch[3]

            key = (srcaddr, messageType, seqNum, dest)
            short_key = (srcaddr, messageType, dest)
            if STAT_CALC == 'Individual' and rxtx == "TXA":
                rxtx = "TX"

            if rxtx == "TX":
                tx[key] += [time]
                if STAT_CALC == "E2E":
                    # if messageType == '03':
                    #     print retx[short_key] if retx.has_key(short_key) else "a"
                    if retx.has_key(short_key):
                        # print retx[short_key]
                        print "poping", short_key
                        for keys in retx[short_key]:
                            if key in keys:
                                keys.remove(key)
                                pass
                        retx[short_key] = filter(lambda (v): v != [], retx[short_key])
                        # retx.pop(short_key)

            if rxtx == "RX" and messageType not in ('0B', '0A'):
                if STAT_CALC == "E2E":
                    if retx.has_key(short_key):
                        print
                        print "Seems like a retransmitted packet was received:"#, key, retx[short_key]
                        # print l.strip()
                        for seqnos in retx[short_key]:
                            if key in seqnos:
                                # print "key change:",key, seqnos[0]
                                key = seqnos[0]
                                # comment next line if first delivery is wanted
                                rx[key] = rx[key][:-1]
                                break

                if len(tx[key]) > len(rx[key]):
                    while len(tx[key]) > len(rx[key]) + 1:
                        # inserindo None para permitir detectar RX repetidos
                        rx[key] += [None]
                    rx[key] += [time]
                elif len(tx[key]) == len(rx[key]):
                    # print tx[key], rx[key], retx[short_key]
                    print "Chave repetida RX", key, "delta t:", time - rx[key][-1]
                else:
                    print "not expected"

            if STAT_CALC == "E2E":
                if rxtx == "TXB":
                    # txb[short_key] = (key, time)
                    txb[short_key] = key

                if rxtx == "TXA":
                    if retx.has_key(short_key):
                        found = False
                        for i in range(len(retx[short_key])):
                            # old
                            # (seqnos, time) = retx[short_key][i]
                            # if seqnos[-1] == txb[short_key][0]:
                            #     retx[short_key][i] = (seqnos + [key], time)

                            # for banana in retx[short_key]:
                            #     print banana
                            # print i
                            if retx[short_key][i][-1] == txb[short_key]:
                                retx[short_key][i] = retx[short_key][i] + [key]
                                txb.pop(short_key)
                                found = True
                                break
                        if not found:
                            # print "did not find", l.strip()
                            retx[short_key] += [ [txb[short_key], key] ]
                    else:
                        retx[short_key] = [ [txb[short_key], key] ]

        # Full Graph
        else:
            s = energy_expression.search(l)
            if s != None:
                time, printid, energytag, energy_left = s.groups()

                timeMatch = timeExpr.match(time)
                # print timeMatch
                timeMatch = map(lambda(x):float(x) if x != None else 0, timeMatch.groups())
                # print timeMatch
                time = 0
                time += timeMatch[0] * 1000 * 60 * 60 if timeMatch[0] != None else 0
                time += timeMatch[1] * 1000 * 60 if timeMatch[1] != None else 0
                time += timeMatch[2] * 1000 if timeMatch[2] != None else 0
                time += timeMatch[3]

                energy[printid] = int(energy_left)
            else:
                s = fg_expression.search(l)
                if s != None:

                    time, printid, FG, NN = s.groups()

                    timeMatch = timeExpr.match(time)
                    # print timeMatch
                    timeMatch = map(lambda(x):float(x) if x != None else 0, timeMatch.groups())
                    # print timeMatch
                    time = 0
                    time += timeMatch[0] * 1000 * 60 * 60 if timeMatch[0] != None else 0
                    time += timeMatch[1] * 1000 * 60 if timeMatch[1] != None else 0
                    time += timeMatch[2] * 1000 if timeMatch[2] != None else 0
                    time += timeMatch[3]

                    if fg_time == -1:
                        fg_time = time
                        # print "FOUND FG"
                    else:
                        print "there should be only one FG"


    sent_type = defaultdict(int)
    sent_node = defaultdict(int)

    received_type = defaultdict(int)
    received_node = defaultdict(int)

    delay_type = defaultdict(float)
    delay_node = defaultdict(float)

    data_delay_all_points = []

    for k,v in tx.iteritems():
        sent_node[k[0]] += len(v)
        sent_type[k[1]] += len(v)
        for (tx_time, rx_time) in zip(v, rx[k]):
            if rx_time != None:
                received_node[k[0]] += 1
                received_type[k[1]] += 1
                delay_node[k[0]] += (rx_time - tx_time)
                delay_type[k[1]] += (rx_time - tx_time)
                if k[1] == '05':
                    data_delay_all_points += [rx_time - tx_time]
                # print rx_time - tx_time, rx_time, tx_time
                if ((rx_time - tx_time) < 0):
                    print "negative time", k, v

    print "Delivery rates by source node:"
    for node in sent_node.keys():
        print '(' + node + ')', "%.2f" % (100.0 * received_node[node] / sent_node[node],), '[', received_node[node], '/', sent_node[node], ']'

    print
    print "Delay by source node:"
    for node in sent_node.keys():
        if received_node[node] != 0:
            print '(' + node + ')', "%.2f" % (delay_node[node] / received_node[node],)
        else:
            print '(' + node + ')', -1

    print
    print "Delivery rates by packet type:"
    for t in sent_type.keys():
        if sent_type[t] != 0:
            print t, "%.2f" % (100.0 * received_type[t] / sent_type[t],), '[', received_type[t], '/', sent_type[t], ']', '\t', packetTypes[int(t,16)]
        else:
            print t, -1

    # f_typecount = file("packet_types_count.txt", 'a')
    # f_typecount.write(";".join(map(packetTypes.get, map(lambda x: int(x, 16), sorted(sent_type.keys())))) + '\n')
    # f_typecount.write(";".join(map(repr, map(sent_type.get, sorted(sent_type.keys())))) + '\n')
    # f_typecount.close()

    if sum(sent_type.values()):
        print "Fraction of data packets (transmitted packets):", "%.2f%%" % (100.0 * sent_type['05'] / sum(sent_type.values()),)
    if sum(received_type.values()):
        print "Fraction of data packets (received packets):", "%.2f%%" % (100.0 * received_type['05'] / sum(received_type.values()),)

    print
    print "Delay by packet type:"
    for t in sent_type.keys():
        if received_type[t] != 0:
            print t, "%.2f" % (delay_type[t] / received_type[t],), '\t', packetTypes[int(t,16)]
        else:
            print t, -1, '\t\t', packetTypes[int(t,16)]

    print
    print "Overall delivery rate (excluding ND and CD):"
    totalSent = sum(map(lambda (k,v): v if k not in ('0A', '0B') else 0, sent_type.iteritems())) #sum(sent_type.values())
    totalRecv = sum(map(lambda (k,v): v if k not in ('0A', '0B') else 0, received_type.iteritems())) #sum(received_type.values())
    print "%.2f" % (100.0 * totalRecv / totalSent,), '[', totalRecv, '/', totalSent, ']'

    if totalRecv:
        print
        print "Overall delay:"
        totalDelay = sum(delay_type.values())
        print "%.2f" % (totalDelay / totalRecv,)

    delivery_data = -1
    delivery_ctrl = -1
    delay_data = -1
    delay_ctrl = -1

    if sent_type['05']:
        delivery_data = 100.0*received_type['05']/sent_type['05']

    if sum(map(lambda (k,v): v if k not in ('05', '0A', '0B') else 0, sent_type.iteritems())):
        delivery_ctrl = 100.0*sum(map(lambda (k,v): v if k not in ('05', '0A', '0B') else 0, received_type.iteritems())) \
                            / sum(map(lambda (k,v): v if k not in ('05', '0A', '0B') else 0, sent_type.iteritems()))

    if received_type['05']:
        delay_data = delay_type['05'] / received_type['05']

    if sum(map(lambda (k,v): v if k not in ('05', '0A', '0B') else 0, received_type.iteritems())):
        delay_ctrl = sum(map(lambda (k,v): v if k not in ('05', '0A', '0B') else 0, delay_type.iteritems())) \
                / sum(map(lambda (k,v): v if k not in ('05', '0A', '0B') else 0, received_type.iteritems()))

    ctrl_overhead = sum(map(lambda (k,v): v if k not in ('05',) else 0, sent_type.iteritems()))

    print
    print "results_summary = (delivery_data, delivery_ctrl, delay_data, delay_ctrl, ctrl_overhead, fg_time, sent_type, data_delay_all_points)"
    results_summary = (delivery_data, delivery_ctrl, delay_data, delay_ctrl, ctrl_overhead, fg_time, sent_type, data_delay_all_points)
    print results_summary
    return results_summary

def pretty_scenario(name):
    pretty_nd = {}
    pretty_nd['CL'] = "Collect"
    pretty_nd['NV'] = "Simple"
    n = repr(name[0])
    topo = name[1]
    if topo.startswith('GRID-'):
        topo = topo [len("GRID-"):]

    nd = pretty_nd[name[2]]

    return "n=%s, %4s, %7s" % (n, topo, nd)
    # return "n=%s" % (n, )


# print len(rx), len(tx)
# print tx
# print rx
# print sent_type
# print received_type
# for k,v in tx.iteritems():
#     print k, v

if __name__ == "__main__":

    if len(sys.argv) == 2:
        parse_file(sys.argv[1])
        exit(0)

    nodes_v = (9, 16, 25)
    # nodes_v = map(lambda x: x*x, range(4,10))
    # nodes_v = (9, 16)
    topologies = ('GRID-FULL', 'GRID-RND', 'GRID-CTA')
    topologies = ('GRID', )
    nd_possibilities = ('CL', 'NV')
    nd_possibilities = ('CL', )
    datarates=(1,)
    nsinkses=(1, 2)
    # nsinkses=(1, )
    MIN_ITER=1
    MAX_ITER=10
    sim_time = 60.0
    results_dir = "results/"
    # results_dir = "./"
    partial_results = defaultdict(list)
    partial_results_packet_count = defaultdict(list)

    results_n = defaultdict(lambda: defaultdict(int))
    results_sum = defaultdict(lambda: defaultdict(int))
    results_sum2 = defaultdict(lambda: defaultdict(int))

    results_packet_count_n = defaultdict(int)
    results_packet_count_sum = defaultdict(lambda: defaultdict(int))
    results_packet_count_sum2 = defaultdict(lambda: defaultdict(int))

    results_avg = defaultdict(dict)
    results_desv = defaultdict(dict)

    results_packet_count_avg = defaultdict(lambda: defaultdict(int))
    results_packet_count_desv = defaultdict(lambda: defaultdict(int))

    data_delay_all_points = defaultdict(list)

    f_all = file("stats_all.txt", 'w')
    f_summary = file("stats_summary.txt", 'w')
    f_typecount = file("packet_types_count.txt", 'w')
    f_typecount.close()

    problematic_files = []

    for nnodes in nodes_v:
        for topo in topologies:
            for nd in nd_possibilities:
                for datarate in datarates:
                    for nsinks in nsinkses:
                        scenario = (nnodes, nsinks, topo, nd, datarate)
                        for i in range(MIN_ITER, MAX_ITER+1):
                            # cooja_ETX_100_ENERGY_0_ITSDN_n81_s1_topGRID_ndCL_i6.txt
                            filename = "cooja_ETX_100_ENERGY_0_ITSDN_n" + repr(nnodes) + "_s" + repr(nsinks) + "_top" + topo + "_nd" + nd + "_d" + repr(datarate) + "_i" + repr(i) + '.txt'
                            filename += 'preproc'
                            try:
                                r = parse_file(results_dir + filename)
                                partial_results[scenario] += [r[:-2]]
                                partial_results_packet_count[scenario] += [r[-2]]
                                data_delay_all_points[scenario].extend(r[-1])
                            except:
                                problematic_files += [filename]
                                print "problem parsing", filename
    print
    print "problematic_files"
    for f in problematic_files:
        print '\t', f
    print

    # plt.rcParams['ps.useafm'] = True
    # plt.rcParams['pdf.use14corefonts'] = True
    # plt.rcParams['text.usetex'] = True
    # plt.rcParams["font.family"] = "Arial"

    metric_names = ("delivery_data", "delivery_ctrl", "delay_data", "delay_ctrl", "ctrl_overhead", "fg_time")
    f_all.write("scenario;")
    f_all.write(";".join(metric_names))
    f_all.write("\n")
    for scenario,result_list in partial_results.iteritems():
        for metric_list in result_list:
            if metric_list == None:
                continue
            f_all.write(repr(scenario) + ";")
            for i_metric in range(len(metric_list)):
                if metric_list[i_metric] > 0:
                    results_n[scenario][i_metric] += 1
                    results_sum[scenario][i_metric] += metric_list[i_metric]
                    results_sum2[scenario][i_metric] += metric_list[i_metric] * metric_list[i_metric]
                    f_all.write(repr(metric_list[i_metric]) + ";")
                else:
                    # results_n[scenario][i_metric] = 0
                    f_all.write("-1;")
            for i_metric in range(len(metric_list)):
                if not results_n[scenario].has_key(i_metric):
                    results_n[scenario][i_metric] = 0
                    results_sum[scenario][i_metric] = 0
                    results_sum2[scenario][i_metric] = 0
            f_all.write("\n")

    for scenario in results_n.keys():
        print scenario
        for i_metric in results_n[scenario].keys():
            c, s, s2 = results_n[scenario][i_metric], results_sum[scenario][i_metric], results_sum2[scenario][i_metric]
            print metric_names[i_metric],
            if c:
                avg = 1.0 * s / c
                print avg,
                results_avg[i_metric][scenario] = avg
            else:
                results_avg[i_metric][scenario] = 0
            if c-1 > 0:
                desv = 1
                if (1.0*s2 - 1.0*s*s/c) > 0:
                    desv = math.sqrt((1.0*s2 - 1.0*s*s/c)/(c-1))
                print desv
                if STAT_DESV == "STD_DEV":
                    results_desv[i_metric][scenario] = desv
                elif STAT_DESV == "CONF_INTERVAL":
                    pvalue = scipy.stats.t.ppf(1 - (1-STAT_CONFIDENCE)/2, c-1)
                    results_desv[i_metric][scenario] = desv * pvalue / math.sqrt(c)
            else:
                print
                results_desv[i_metric][scenario] = 0


    for i_metric in range(len(metric_names)):
        print "writing to file", metric_names[i_metric]
        f_summary.write(repr(metric_names[i_metric]) + '\n')
        f_summary.write('scenario;average;std deviation\n')
        for scenario in sorted(results_avg[i_metric].keys()):
            f_summary.write(repr(scenario) + ";")
            f_summary.write(repr(results_avg[i_metric][scenario]) + ";")
            f_summary.write(repr(results_desv[i_metric][scenario]) + ";\n")
        f_summary.write("\n")

    f_all.close()
    f_summary.close()

    for scenario,count_dicts in partial_results_packet_count.iteritems():
        for count_dict in count_dicts:
            results_packet_count_n[scenario] += 1
            # print count_dict
            for pkt_type, count in count_dict.iteritems():
                results_packet_count_sum[scenario][pkt_type] += count
                results_packet_count_sum2[scenario][pkt_type] += count * count

    for scenario in results_packet_count_n.keys():
        # print scenario
        c = results_packet_count_n[scenario]

        for pkt_type in results_packet_count_sum[scenario].keys():
            s = results_packet_count_sum[scenario][pkt_type]
            s2 = results_packet_count_sum2[scenario][pkt_type]
            if c:
                avg = 1.0 * s / c
                results_packet_count_avg[scenario][pkt_type] = avg
            else:
                results_packet_count_avg[scenario][pkt_type] = 0

            if c-1 > 0:
                desv = 1
                if (1.0*s2 - 1.0*s*s/c) > 0:
                    desv = math.sqrt((1.0*s2 - 1.0*s*s/c)/(c-1))
                if STAT_DESV == "STD_DEV":
                    results_packet_count_desv[scenario][pkt_type] = desv
                elif STAT_DESV == "CONF_INTERVAL":
                    pvalue = scipy.stats.t.ppf(1 - (1-STAT_CONFIDENCE)/2, c-1)
                    results_packet_count_desv[scenario][pkt_type] = desv * pvalue / math.sqrt(c)
            else:
                results_packet_count_desv[i_metric][scenario] = 0


    packet_types_labels = {
    3: 'Flow request',
    4: 'Neighbor report',
    5: 'Data packet',
    6 + 0xE0 : 'SDN_PACKET_SRC_ROUTED_CONTROL_FLOW_SETUP',
    7 + 0xE0: 'Flow setup',
    8 + 0xD0: 'SDN_PACKET_MULTIPLE_CONTROL_FLOW_SETUP',
    9 + 0xD0: 'SDN_PACKET_MULTIPLE_DATA_FLOW_SETUP',
    10: 'Neighbor discovery',
    11: 'SDN_PACKET_CD',
    12: 'Node ACK',
    13: 'Controller ACK',
    14: 'Register flowId',
    15: 'SDN_PACKET_ENERGY_REPORT',
    16: 'SDN_PACKET_ACK_BY_FLOW_ADDRESS'
    }
    plt.rcParams.update({'font.size': 16, 'legend.fontsize': 14})

    for s, r in sorted(results_packet_count_avg.iteritems()):
        print s, r["0A"]
    print
    scenarios = sorted(results_packet_count_avg.keys())

    f = file("packet_count.txt",'w')
    k = sorted(results_packet_count_avg[scenarios[0]].keys())
    f.write("scenario;")
    f.write(";".join(map(packet_types_labels.get, map(lambda x: int(x,16), k))))
    f.write("\n")

    for scenario in scenarios:
        k = sorted(results_packet_count_avg[scenario].keys())
        v = map(results_packet_count_avg[scenario].get, k)
        v = map(repr, v)
        f.write(repr(scenario))
        f.write(";")
        f.write(";".join(v))
        f.write("\n")
    f.close()

    scenarios = filter(lambda x: (x[0] == 64 or x[0] == 25 or x[0] == 810) and (x[1] == 1 or x[1] == 2) , scenarios)
    # print "plotting"

    colors = ("#ffffb2", "#fdae6b", "#c7e9c0", "#de2d26", "#9e9ac8", "#353535")
    colors = ("#ffffb2", "#D88944", "#c7e9c0", "#de2d26", "#9e9ac8", "#353535")

    lss = ['solid', 'dashed', 'dotted', 'dashdot', '-', '--', '-.', ':', 'None', ' ', '']
    # fig, ax = plt.subplots(figsize=(22,5))
    fig, ax = plt.subplots()
    n = 0
    myhatch = ""
    savetype = 'png'

    for scenario in scenarios:
        width = 0.2
        ind = np.arange(len(results_packet_count_avg[scenario]))

        k = sorted(results_packet_count_avg[scenario].keys())
        v = map(results_packet_count_avg[scenario].get, k)
        e = map(results_packet_count_desv[scenario].get, k)
        e = map(lambda x: 1.0*x / sum(v), e)
        v = map(lambda x: 1.0*x / sum(v), v)

        # ax.bar(ind + width*n, v, width, color=colors[n%len(colors)], yerr=e, ecolor='black', label=pretty_name(name) )
        pretty_name = repr(scenario[0]) + " nodes, " + repr(scenario[1]) + (" sinks" if scenario[1] > 1 else " sink")
        ax.bar(ind + width*n, v, width, color=colors[(n/2)%len(colors)], yerr=e, ecolor='black', label = pretty_name, hatch=myhatch)
        if myhatch == "":
            myhatch = '//'
        else:
            myhatch = ""
        n+=1

        # print scenario

    plt.xticks(ind + width * len(scenarios) / 2, map(packet_types_labels.get, map(lambda x: int(x,16), k)), rotation=30, ha='right', rotation_mode='anchor' )
    plt.grid(b=True, which='major', color='gray', linestyle='--', lw=0.1, axis='y')
    # plt.ylim(0.1, 10**5)
    # ax.set_yscale("log")
    plt.legend(loc='best', ncol=1)
    ax.set_xlabel('Packet type')
    ax.set_ylabel('Number of packets')
    ax.set_ylabel('Number of packets [\%]')
    plt.tight_layout()
    # plt.savefig('packet_count.eps', format='eps', dpi=1000, bbox_inches='tight')
    plt.savefig('packet_count_relative.' + savetype, format=savetype, dpi=300, bbox_inches='tight')

    colors = ("red", "green", "gray", "cyan", "orange", "pink")
    colors = ("#000099", "#ff9933", "#4c9900")

    scenarios = sorted(data_delay_all_points.keys())
    # scenarios = filter(lambda x: (x[0] == 36 or x[0] == 81) and (x[1] == 1 or x[1] == 2) , scenarios)
    scenarios = filter(lambda x: (x[0] == 49 or (x[0] == 81 and x[1] == 1 )) , scenarios)
    index = 0
    fig, ax = plt.subplots()
    pct = 95.0
    pct_vs = []
    for scenario in scenarios:

        pretty_name = repr(scenario[0]) + " nodes, " + repr(scenario[1]) + " sink"
        X = map(lambda x: x/1000, data_delay_all_points[scenario])

        # plot the cumulative histogram
        # n, bins, patches = ax.hist(X, bins="auto", normed=1, histtype='step', cumulative=True, label=pretty_name)
        # patches[0].set_xy(patches[0].get_xy()[:-1])

        n = np.arange(1,len(X)+1) / np.float(len(X))
        Xs = np.sort(X)
        plt.step(Xs, n, label=pretty_name, color=colors[index], linewidth=4.0, linestyle=lss[index])

        pct_v = np.percentile(X, pct)
        pct_vs += [pct_v]
        print scenario, np.percentile(X, pct)
        lines = plt.plot((pct_v, pct_v), (0, pct/100), "--")
        plt.setp(lines, linewidth=0.5, color=colors[index])

        index += 1

    # tidy up the figure
    ax.grid(True)
    ax.legend(loc='best')
    ax.set_xlabel('Data delay [ms]')
    ax.set_ylabel('Cumulative probability')
    ax.set_xscale("log")
    ax.set_xlim(1, 10**4)
    # ax.set_yscale("log")
    # ax.set_ylim(0.0000, 1.1)

    # plt.xticks(sorted(list(plt.xticks()[0]) + pct_vs))
    # addticks(ax, pct_vs, pct_vs)
    lines = plt.plot((0, max(pct_vs)), (pct/100, pct/100), "--")
    plt.setp(lines, linewidth=1.0, color="gray")
    plt.savefig('r_delay_cdf.'+savetype, format=savetype, dpi=300, bbox_inches='tight')


    exit()

    ##########################################################################
    colors = ("red", "green", "cyan", "gray", "orange", "pink")
    colors = ("#d73027", "#fc8d59", "#fee090", "#e0f3f8", "#91bfdb", "#4575b4")
    colors = ("#f7f7f7", "#d9d9d9", "#bdbdbd", "#969696", "#636363", "#252525")
    colors = ("#f7f7f7", "#bdbdbd", "#d9d9d9", "#636363", "#969696", "#353535")
    colors = ("#ffffb2", "#fdae6b", "#c7e9c0", "#de2d26", "#9e9ac8", "#353535")
    # colors = ("#ffffb2", "#fdae6b", "#c7e9c0")
    hatches = ('//', '', '\\\\', 'o', '-', '+', 'x', '*', 'O', '.', '/', '\\')
    plt.rcParams.update({'font.size': 16, 'legend.fontsize': 14})
    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)
    plt.grid(b=True, which='major', color='gray', linestyle='--', lw=0.1, axis='y')
    print
    print "ploting delivery"
    i_metric = 0
    bar_width = 0.8/len(results_avg[i_metric].keys())
    x = np.arange(2)
    i = 0
    for scenario in sorted(results_avg[i_metric].keys()):
        y = (results_avg[i_metric][scenario], results_avg[i_metric+1][scenario])
        desv = (results_desv[i_metric][scenario], results_desv[i_metric+1][scenario])
        print x + bar_width*i, y, bar_width
        plt.bar(x + bar_width*(i + i/len(colors)/2.0), y, bar_width, yerr=desv, label=pretty_scenario(scenario), color=colors[i%len(colors)], hatch=hatches[i/len(colors)], ecolor='black' )
        i+=1
        # print x
        # print y
    # plt.errorbar(x, y, e, ls=lss[rc], color=colors[rc], marker=markers[rc])
    plt.xlabel('Packet Type')
    plt.ylabel('Delivery [%]')
    plt.xticks(x + bar_width * len(results_avg[i_metric].keys()) / 2, ('Data', 'Control'))
    plt.legend(ncol=3)
    axes = plt.gca()
    axes.set_xlim([-bar_width/2, 1 + bar_width*(i + i/len(colors)/2.0) ])

    plt.savefig('r_delivery.eps', format='eps', dpi=1000)

    ##########################################################################
    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)
    plt.grid(b=True, which='major', color='gray', linestyle='--', lw=0.1, axis='y')
    ax.set_yscale('log')#, basex=2)
    print
    print "ploting delay"
    i_metric = 2
    bar_width = 0.8/len(results_avg[i_metric].keys())
    x = np.arange(2)
    i = 0
    for scenario in sorted(results_avg[i_metric].keys()):
        y = (results_avg[i_metric][scenario], results_avg[i_metric+1][scenario])
        y = map(lambda v: v/1000000.0, y)
        desv = (results_desv[i_metric][scenario], results_desv[i_metric+1][scenario])
        desv = map(lambda v: v/1000000.0, desv)
        print x + bar_width*i, y, bar_width
        try:
            plt.bar(x + bar_width*(i + i/len(colors)/2.0), y, bar_width, yerr=desv, label=pretty_scenario(scenario), color=colors[i%len(colors)], hatch=hatches[i/len(colors)], ecolor='black' )
        except:
            pass
        i+=1
        # print x
        # print y
    # plt.errorbar(x, y, e, ls=lss[rc], color=colors[rc], marker=markers[rc])
    plt.xlabel('Packet Type')
    plt.ylabel('Delay [s]')
    plt.xticks(x + bar_width * len(results_avg[i_metric].keys()) / 2, ('Data', 'Control'))
    plt.legend(ncol=3)
    axes = plt.gca()
    axes.set_xlim([-bar_width/2, 1 + bar_width*(i + i/len(colors)/2.0) ])

    plt.savefig('r_delay.eps', format='eps', dpi=1000)

    ##########################################################################
    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)
    plt.grid(b=True, which='major', color='gray', linestyle='--', lw=0.1, axis='y')
    print
    print "ploting overhead"
    i_metric = 4
    bar_width = 0.4/len(results_avg[i_metric].keys())
    x = np.arange(1)
    i = 0
    for scenario in sorted(results_avg[i_metric].keys()):
        y = (results_avg[i_metric][scenario] )
        y = y / sim_time
        y = y / scenario[0]
        desv = (results_desv[i_metric][scenario] )
        desv = desv / sim_time
        desv = desv / scenario[0]
        print x + bar_width*i, y, bar_width
        plt.bar(x + bar_width*(i + i/len(colors)), y, bar_width, yerr=desv, label=pretty_scenario(scenario), color=colors[i%len(colors)], hatch=hatches[i/len(colors)], ecolor='black' )
        i+=1
        # print x
        # print y
    # plt.errorbar(x, y, e, ls=lss[rc], color=colors[rc], marker=markers[rc])
    plt.xlabel('')
    plt.ylabel('Control Overhead [pkts/min/node]')
    plt.xticks(x + bar_width * len(results_avg[i_metric].keys()) / 2, ('', ))
    plt.legend(ncol=3)
    axes = plt.gca()
    axes.set_xlim(-bar_width/2)
    axes.set_ylim( (0, 3) )

    plt.savefig('r_overhead.eps', format='eps', dpi=1000)

    ##########################################################################
    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)
    plt.grid(b=True, which='major', color='gray', linestyle='--', lw=0.1, axis='y')
    print
    print "ploting time to full network"
    i_metric = 5
    bar_width = 0.6/len(results_avg[i_metric].keys())
    x = np.arange(1)
    i = 0
    # legends = []
    for scenario in sorted(results_avg[i_metric].keys()):
        # legends += [scenario]
        y = (results_avg[i_metric][scenario] )
        y = y/1000000.0
        desv = (results_desv[i_metric][scenario] )
        desv = desv/1000000.0
        print x + bar_width*i, y, bar_width
        plt.bar(x + bar_width*(i + i/len(colors)), y, bar_width, yerr=desv, label=pretty_scenario(scenario), color=colors[i%len(colors)], hatch=hatches[i/len(colors)], ecolor='black' )
        i+=1
        # print x
        # print y
    # plt.errorbar(x, y, e, ls=lss[rc], color=colors[rc], marker=markers[rc])
    plt.xlabel('')
    plt.ylabel('Time to controller see all network [s]')
    axes = plt.gca()
    axes.set_ylim([0, 500])
    plt.xticks(x + bar_width * len(results_avg[i_metric].keys()) / 2, ('', ))
    lgd = plt.legend(ncol=3)
    plt.savefig('r_fullnetwork.eps', format='eps', dpi=1000)

    # legends = map(pretty_scenario, legends)
    lgd = plt.legend( bbox_to_anchor=(-4,4), ncol=6)
    plt.savefig('r_legend.eps', format='eps', dpi=1000, bbox_inches='tight', bbox_extra_artists=(lgd,) if lgd != None else None)

    # for i_metric in range(len(metric_names)):
    #     if i in (0, 2, 4, 5):
    #         fig = plt.figure()
    #         ax = fig.add_subplot(1, 1, 1)
    #     print
    #     print "ploting", metric_names[i_metric]
    #     # x,y,e = map_to_xye(all_result_lines[rc], i)
    #     x = np.arange(len(results_avg[i_metric].values()))
    #     y = results_avg[i_metric].values()
    #     desv = results_desv[i_metric].values()
    #     print x
    #     print y
    #     # plt.errorbar(x, y, e, ls=lss[rc], color=colors[rc], marker=markers[rc])
    #     plt.bar(x, y, yerr=desv)
    #     plt.savefig('destination_path'+repr(i_metric)+'.eps', format='eps', dpi=1000)
