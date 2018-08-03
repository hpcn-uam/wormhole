package backtype.storm.tuple;

public class Tuple //extends WorkerTopologyContext //implements IMetricsContext
{
	String whstring;

	public Tuple()
	{

	}

	public int size()
	{
		return 1;
	}

	public String getString(int i)
	{
		if (i == 0) {
			return whstring;

		} else {
			return null;    //TODO throw exception else
		}
	}

	public void setWHString(String whstring)
	{
		this.whstring = whstring;
	}
}