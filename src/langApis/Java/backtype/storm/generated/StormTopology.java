package backtype.storm.generated;

import es.hpcn.wormhole.Worm;
import es.hpcn.wormhole.Einstein;

public class StormTopology
{
	public StormTopology()
	{

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