package backtype.storm.topology.base;

import backtype.storm.topology.IRichSpout;

import java.util.Map;

public class BaseRichSpout implements IRichSpout
{
	public BaseRichSpout()
	{

	}

	public Map<String, Object> getComponentConfiguration()
	{
		return null;
	}
}