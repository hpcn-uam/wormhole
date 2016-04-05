package backtype.storm.topology;

import backtype.storm.generated.StormTopology;

import java.util.ArrayList;

public class TopologyBuilder
{
	ArrayList<IRichSpout> spouts;
	ArrayList<IRichBolt> bolts;

	public TopologyBuilder()
	{
		spouts = new ArrayList<IRichSpout>();
		bolts  = new ArrayList<IRichBolt> ();
	}

	public SpoutDeclarer setSpout(String id, IRichSpout spout, Number parallelism_hint)
	{
		spouts.add(spout);
		return new SpoutDeclarer();
	}

	public  BoltDeclarer setBolt(String id, IRichBolt bolt, Number parallelism_hint)
	{
		bolts.add(bolt);
		return new BoltDeclarer();
	}

	public StormTopology createTopology()
	{
		StormTopology ret = new StormTopology();
		ret.WHsetSpoutsAndBolts(spouts, bolts);
		return ret;
	}

}