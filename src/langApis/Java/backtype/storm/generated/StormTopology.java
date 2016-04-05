package backtype.storm.generated;

import es.hpcn.wormhole.Worm;
import es.hpcn.wormhole.Einstein;

import backtype.storm.topology.IRichBolt;
import backtype.storm.topology.IRichSpout;

import java.util.ArrayList;

public class StormTopology
{
	private ArrayList<IRichSpout> spouts;
	private ArrayList<IRichBolt> bolts;

	public StormTopology()
	{

	}

	public void WHsetSpoutsAndBolts(ArrayList<IRichSpout> spouts, ArrayList<IRichBolt> bolts)
	{
		this.spouts = spouts;
		this.bolts = bolts;
	}

	public ArrayList<IRichSpout> WHgetSpouts()
	{
		return spouts;
	}

	public ArrayList<IRichBolt> WHgetBolts()
	{
		return bolts;
	}

	public String getWHconfig(int wormId)
	{
		if (wormId == 1) {
			return wormId + " javaTest localhost -1";
			//return wormId + " testBW 192.168.50.103 0x3F";

		} else {
			return wormId + " javaTest localhost -1";
			//return wormId + " testBW 192.168.50.103 0x3F";
		}
	}

	public String getWHdesc(int wormId)
	{
		if (wormId == 1) {
			return "2";

		} else {
			return "1";
		}
	}

	public int getWHnum()
	{
		return 2;
	}
}