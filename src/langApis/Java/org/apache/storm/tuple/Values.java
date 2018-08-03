package backtype.storm.tuple;

import java.util.ArrayList;

public class Values extends ArrayList<Object>
{
	public Values()
	{

	}

	public Values(Object data)
	{
		this.add(data);
	}
}