package backtype.storm;

import es.hpcn.wormhole.Worm;
import es.hpcn.wormhole.Einstein;

public class TopologyBuilder
{
	public TopologyBuilder()
	{

	}

	public SpoutDeclarer setSpout(String id, IRichSpout spout, Number parallelism_hint)
	{
		return new SpoutDeclarer();
	}

	public  BoltDeclarer	setBolt(String id, IRichBolt bolt, Number parallelism_hint)
	{
		return new BoltDeclarer();
	}
}