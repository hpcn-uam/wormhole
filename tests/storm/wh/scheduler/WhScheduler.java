/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * <p>
 * http://www.apache.org/licenses/LICENSE-2.0
 * <p>
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// This file was taken from https://raw.githubusercontent.com/apache/storm/v1.0.6/storm-core/src/jvm/org/apache/storm/scheduler/resource/ResourceAwareScheduler.java

package wh.scheduler;

import org.apache.storm.scheduler.resource.*;

import org.apache.storm.Config;
import org.apache.storm.scheduler.resource.strategies.eviction.IEvictionStrategy;
import org.apache.storm.scheduler.resource.strategies.priority.ISchedulingPriorityStrategy;
import org.apache.storm.scheduler.resource.strategies.scheduling.IStrategy;
import org.apache.storm.utils.Utils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.apache.storm.scheduler.Cluster;
import org.apache.storm.scheduler.ExecutorDetails;
import org.apache.storm.scheduler.IScheduler;
import org.apache.storm.scheduler.Topologies;
import org.apache.storm.scheduler.TopologyDetails;
import org.apache.storm.scheduler.WorkerSlot;

import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.ArrayList;

public class WhScheduler implements IScheduler {

    // Object that holds the current scheduling state
    private SchedulingState schedulingState;

    @SuppressWarnings("rawtypes")
    private Map conf;

    private static final Logger LOG = LoggerFactory.getLogger(WhScheduler.class);

    @Override
    public void prepare(Map conf) {
        this.conf = conf;

    }

    @Override
    public void schedule(Topologies topologies, Cluster cluster) {
        LOG.debug("\n\n\nRerunning WhScheduler...");
        // initialize data structures
        initialize(topologies, cluster);
        // logs everything that is currently scheduled and the location at which they
        // are scheduled
        LOG.info("Cluster scheduling:\n{}", ResourceUtils.printScheduling(cluster, topologies));
        // logs the resources available/used for every node
        LOG.info("Nodes:\n{}", this.schedulingState.nodes);
        // logs the detailed info about each user
        for (User user : getUserMap().values()) {
            LOG.info(user.getDetailedInfo());
        }

        ISchedulingPriorityStrategy schedulingPrioritystrategy = null;
        while (true) {

            if (schedulingPrioritystrategy == null) {
                try {
                    schedulingPrioritystrategy = (ISchedulingPriorityStrategy) Utils
                            .newInstance((String) this.conf.get(Config.RESOURCE_AWARE_SCHEDULER_PRIORITY_STRATEGY));
                } catch (RuntimeException ex) {
                    LOG.error(String.format(
                            "failed to create instance of priority strategy: %s with error: %s! No topologies will be scheduled.",
                            this.conf.get(Config.RESOURCE_AWARE_SCHEDULER_PRIORITY_STRATEGY), ex.getMessage()), ex);
                    break;
                }
            }
            TopologyDetails td;
            try {
                // need to re prepare since scheduling state might have been restored
                schedulingPrioritystrategy.prepare(this.schedulingState);
                // Call scheduling priority strategy
                td = schedulingPrioritystrategy.getNextTopologyToSchedule();
            } catch (Exception ex) {
                LOG.error(String.format(
                        "Exception thrown when running priority strategy %s. No topologies will be scheduled! Error: %s",
                        schedulingPrioritystrategy.getClass().getName(), ex.getMessage()),
                        ex.getStackTrace().toString());
                break;
            }
            if (td == null) {
                break;
            }
            scheduleTopology(cluster, td);

            LOG.debug("Nodes after scheduling:\n{}", this.schedulingState.nodes);
        }

        // update changes to cluster
        updateChanges(cluster, topologies);
    }

    private void updateChanges(Cluster cluster, Topologies topologies) {
        // Cannot simply set this.cluster=schedulingState.cluster since clojure is
        // immutable
        cluster.setAssignments(schedulingState.cluster.getAssignments());
        cluster.setBlacklistedHosts(schedulingState.cluster.getBlacklistedHosts());
        cluster.setStatusMap(schedulingState.cluster.getStatusMap());
        cluster.setSupervisorsResourcesMap(schedulingState.cluster.getSupervisorsResourcesMap());
        cluster.setTopologyResourcesMap(schedulingState.cluster.getTopologyResourcesMap());
        // updating resources used by supervisor
        updateSupervisorsResources(cluster, topologies);
    }

    public void scheduleTopology(Cluster cluster, TopologyDetails td) {
        User topologySubmitter = this.schedulingState.userMap.get(td.getTopologySubmitter());

        for (RAS_Node targetNode : this.schedulingState.nodes.getNodes()) {
            LOG.debug("Hello from: {}", targetNode.getHostname());

            // WorkerSlot targetSlot = null;
            Collection<ExecutorDetails> exdet = cluster.getUnassignedExecutors(td);

            for (ExecutorDetails ex : exdet) {
                boolean rebreak = false;
                for (WorkerSlot targetSlot : targetNode.getFreeSlots()) {
                    if (targetSlot != null) {
                        String targetHost = "";
                        ArrayList<ExecutorDetails> toAssign = new ArrayList<ExecutorDetails>();

                        if (ex.getStartTask() == 1) {
                            targetHost = "mc1.magnesio.ii.uam.es";
                            toAssign.add(ex);
                        } else if (ex.getStartTask() == 2) {
                            targetHost = "compe1.magnesio.ii.uam.es";
                            toAssign.add(ex);
                        } else if (ex.getStartTask() == 3) {
                            targetHost = "compe1.magnesio.ii.uam.es";
                            toAssign.add(ex);
                        }

                        LOG.debug("POLICY: {} - {} - {}", targetNode, targetHost, ex.getStartTask());

                        if (targetNode.getHostname().equals(targetHost)) {
                            targetNode.assign(targetSlot, td, toAssign);
                            LOG.debug("ASSIGNMENT    TOPOLOGY: {}  TASKS: {} To Node: {} on Slot: {}", td.getName(),
                                    exdet, targetNode.getHostname(), targetSlot.getPort());
                            // rebreak = true;
                            break;
                        }
                    }
                }
                if (rebreak) {
                    break;
                }
            }
        }

        topologySubmitter.moveTopoFromPendingToRunning(td);

        // if (this.schedulingState.cluster.getUnassignedExecutors(td).size() > 0) {
        // LOG.debug("/********Scheduling topology {} from User {}************/",
        // td.getName(), topologySubmitter);

        // SchedulingState schedulingState = checkpointSchedulingState();
        // IEvictionStrategy evictionStrategy = null;
        // while (true) {
        // SchedulingResult result = null;
        // LOG.debug("scheduling result: {}", result);
        // if (result != null && result.isValid()) {
        // if (result.isSuccess()) {
        // try {
        // if (mkAssignment(td, result.getSchedulingResultMap())) {
        // topologySubmitter.moveTopoFromPendingToRunning(td);
        // this.schedulingState.cluster.setStatus(td.getId(), "Running - " +
        // result.getMessage());
        // } else {
        // topologySubmitter = this.cleanup(schedulingState, td);
        // topologySubmitter.moveTopoFromPendingToAttempted(td);
        // this.schedulingState.cluster.setStatus(td.getId(),
        // "Unsuccessful in scheduling - Unable to assign executors to nodes. Please
        // check logs for details");
        // }
        // }
        // break;
        // }
        // } else {
        // LOG.warn(
        // "Scheduling results returned from topology {} is not vaild! Topology with be
        // ignored.",
        // td.getName());
        // topologySubmitter = cleanup(schedulingState, td);
        // topologySubmitter.moveTopoFromPendingToInvalid(td,
        // this.schedulingState.cluster);
        // break;
        // }
        // }
        // } else {
        // LOG.warn("Topology {} is already fully scheduled!", td.getName());
        // topologySubmitter.moveTopoFromPendingToRunning(td);
        // if (this.schedulingState.cluster.getStatusMap().get(td.getId()) == null
        // || this.schedulingState.cluster.getStatusMap().get(td.getId()).equals("")) {
        // this.schedulingState.cluster.setStatus(td.getId(), "Fully Scheduled");
        // }
        // }
    }

    private User cleanup(SchedulingState schedulingState, TopologyDetails td) {
        restoreCheckpointSchedulingState(schedulingState);
        // since state is restored need the update User topologySubmitter to the new
        // User object in userMap
        return this.schedulingState.userMap.get(td.getTopologySubmitter());
    }

    private boolean mkAssignment(TopologyDetails td,
            Map<WorkerSlot, Collection<ExecutorDetails>> schedulerAssignmentMap) {
        if (schedulerAssignmentMap != null) {
            double requestedMemOnHeap = td.getTotalRequestedMemOnHeap();
            double requestedMemOffHeap = td.getTotalRequestedMemOffHeap();
            double requestedCpu = td.getTotalRequestedCpu();
            double assignedMemOnHeap = 0.0;
            double assignedMemOffHeap = 0.0;
            double assignedCpu = 0.0;

            Set<String> nodesUsed = new HashSet<String>();
            for (Map.Entry<WorkerSlot, Collection<ExecutorDetails>> workerToTasksEntry : schedulerAssignmentMap
                    .entrySet()) {
                WorkerSlot targetSlot = workerToTasksEntry.getKey();
                Collection<ExecutorDetails> execsNeedScheduling = workerToTasksEntry.getValue();
                RAS_Node targetNode = this.schedulingState.nodes.getNodeById(targetSlot.getNodeId());

                targetSlot = allocateResourceToSlot(td, execsNeedScheduling, targetSlot);

                targetNode.assign(targetSlot, td, execsNeedScheduling);

                LOG.debug("ASSIGNMENT    TOPOLOGY: {}  TASKS: {} To Node: {} on Slot: {}", td.getName(),
                        execsNeedScheduling, targetNode.getHostname(), targetSlot.getPort());

                for (ExecutorDetails exec : execsNeedScheduling) {
                    targetNode.consumeResourcesforTask(exec, td);
                }
                if (!nodesUsed.contains(targetNode.getId())) {
                    nodesUsed.add(targetNode.getId());
                }
                assignedMemOnHeap += targetSlot.getAllocatedMemOnHeap();
                assignedMemOffHeap += targetSlot.getAllocatedMemOffHeap();
                assignedCpu += targetSlot.getAllocatedCpu();
            }

            Double[] resources = { requestedMemOnHeap, requestedMemOffHeap, requestedCpu, assignedMemOnHeap,
                    assignedMemOffHeap, assignedCpu };
            LOG.debug(
                    "setTopologyResources for {}: requested on-heap mem, off-heap mem, cpu: {} {} {} "
                            + "assigned on-heap mem, off-heap mem, cpu: {} {} {}",
                    td.getId(), requestedMemOnHeap, requestedMemOffHeap, requestedCpu, assignedMemOnHeap,
                    assignedMemOffHeap, assignedCpu);
            // updating resources used for a topology
            this.schedulingState.cluster.setTopologyResources(td.getId(), resources);
            return true;
        } else {
            LOG.warn("schedulerAssignmentMap for topo {} is null. This shouldn't happen!", td.getName());
            return false;
        }
    }

    private WorkerSlot allocateResourceToSlot(TopologyDetails td, Collection<ExecutorDetails> executors,
            WorkerSlot slot) {
        double onHeapMem = 0.0;
        double offHeapMem = 0.0;
        double cpu = 0.0;
        for (ExecutorDetails exec : executors) {
            Double onHeapMemForExec = td.getOnHeapMemoryRequirement(exec);
            if (onHeapMemForExec != null) {
                onHeapMem += onHeapMemForExec;
            }
            Double offHeapMemForExec = td.getOffHeapMemoryRequirement(exec);
            if (offHeapMemForExec != null) {
                offHeapMem += offHeapMemForExec;
            }
            Double cpuForExec = td.getTotalCpuReqTask(exec);
            if (cpuForExec != null) {
                cpu += cpuForExec;
            }
        }
        return new WorkerSlot(slot.getNodeId(), slot.getPort(), onHeapMem, offHeapMem, cpu);
    }

    private void updateSupervisorsResources(Cluster cluster, Topologies topologies) {
        Map<String, Double[]> supervisors_resources = new HashMap<String, Double[]>();
        Map<String, RAS_Node> nodes = RAS_Nodes.getAllNodesFrom(cluster, topologies);
        for (Map.Entry<String, RAS_Node> entry : nodes.entrySet()) {
            RAS_Node node = entry.getValue();
            Double totalMem = node.getTotalMemoryResources();
            Double totalCpu = node.getTotalCpuResources();
            Double usedMem = totalMem - node.getAvailableMemoryResources();
            Double usedCpu = totalCpu - node.getAvailableCpuResources();
            Double[] resources = { totalMem, totalCpu, usedMem, usedCpu };
            supervisors_resources.put(entry.getKey(), resources);
        }
        cluster.setSupervisorsResourcesMap(supervisors_resources);
    }

    public User getUser(String user) {
        return this.schedulingState.userMap.get(user);
    }

    public Map<String, User> getUserMap() {
        return this.schedulingState.userMap;
    }

    /**
     * Intialize scheduling and running queues
     *
     * @param topologies
     * @param cluster
     */
    private Map<String, User> getUsers(Topologies topologies, Cluster cluster) {
        Map<String, User> userMap = new HashMap<String, User>();
        Map<String, Map<String, Double>> userResourcePools = getUserResourcePools();
        LOG.debug("userResourcePools: {}", userResourcePools);

        for (TopologyDetails td : topologies.getTopologies()) {

            String topologySubmitter = td.getTopologySubmitter();
            // additional safety check to make sure that topologySubmitter is going to be a
            // valid value
            if (topologySubmitter == null || topologySubmitter.equals("")) {
                LOG.error("Cannot determine user for topology {}.  Will skip scheduling this topology", td.getName());
                continue;
            }
            if (!userMap.containsKey(topologySubmitter)) {
                userMap.put(topologySubmitter, new User(topologySubmitter, userResourcePools.get(topologySubmitter)));
            }
            if (cluster.getUnassignedExecutors(td).size() > 0) {
                LOG.debug("adding td: {} to pending queue", td.getName());
                userMap.get(topologySubmitter).addTopologyToPendingQueue(td);
            } else {
                LOG.debug("adding td: {} to running queue with existing status: {}", td.getName(),
                        cluster.getStatusMap().get(td.getId()));
                userMap.get(topologySubmitter).addTopologyToRunningQueue(td);
                if (cluster.getStatusMap().get(td.getId()) == null
                        || cluster.getStatusMap().get(td.getId()).equals("")) {
                    cluster.setStatus(td.getId(), "Fully Scheduled");
                }
            }
        }
        return userMap;
    }

    private void initialize(Topologies topologies, Cluster cluster) {
        Map<String, User> userMap = getUsers(topologies, cluster);
        this.schedulingState = new SchedulingState(userMap, cluster, topologies, this.conf);
    }

    /**
     * Get resource guarantee configs
     *
     * @return a map that contains resource guarantees of every user of the
     *         following format {userid->{resourceType->amountGuaranteed}}
     */
    private Map<String, Map<String, Double>> getUserResourcePools() {
        Object raw = this.conf.get(Config.RESOURCE_AWARE_SCHEDULER_USER_POOLS);
        Map<String, Map<String, Double>> ret = new HashMap<String, Map<String, Double>>();

        if (raw != null) {
            for (Map.Entry<String, Map<String, Number>> userPoolEntry : ((Map<String, Map<String, Number>>) raw)
                    .entrySet()) {
                String user = userPoolEntry.getKey();
                ret.put(user, new HashMap<String, Double>());
                for (Map.Entry<String, Number> resourceEntry : userPoolEntry.getValue().entrySet()) {
                    ret.get(user).put(resourceEntry.getKey(), resourceEntry.getValue().doubleValue());
                }
            }
        }

        Map fromFile = Utils.findAndReadConfigFile("user-resource-pools.yaml", false);
        Map<String, Map<String, Number>> tmp = (Map<String, Map<String, Number>>) fromFile
                .get(Config.RESOURCE_AWARE_SCHEDULER_USER_POOLS);
        if (tmp != null) {
            for (Map.Entry<String, Map<String, Number>> userPoolEntry : tmp.entrySet()) {
                String user = userPoolEntry.getKey();
                ret.put(user, new HashMap<String, Double>());
                for (Map.Entry<String, Number> resourceEntry : userPoolEntry.getValue().entrySet()) {
                    ret.get(user).put(resourceEntry.getKey(), resourceEntry.getValue().doubleValue());
                }
            }
        }
        return ret;
    }

    private SchedulingState checkpointSchedulingState() {
        LOG.debug("/*********Checkpoint scheduling state************/");
        for (User user : this.schedulingState.userMap.values()) {
            LOG.debug(user.getDetailedInfo());
        }
        LOG.debug(ResourceUtils.printScheduling(this.schedulingState.cluster, this.schedulingState.topologies));
        LOG.debug("nodes:\n{}", this.schedulingState.nodes);
        LOG.debug("/*********End************/");
        return new SchedulingState(this.schedulingState);
    }

    private void restoreCheckpointSchedulingState(SchedulingState schedulingState) {
        LOG.debug("/*********restoring scheduling state************/");
        // reseting cluster
        this.schedulingState = schedulingState;
        for (User user : this.schedulingState.userMap.values()) {
            LOG.debug(user.getDetailedInfo());
        }
        LOG.debug(ResourceUtils.printScheduling(this.schedulingState.cluster, this.schedulingState.topologies));
        LOG.debug("nodes:\n{}", this.schedulingState.nodes);
        LOG.debug("/*********End************/");
    }
}