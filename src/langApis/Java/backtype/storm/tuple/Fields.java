package backtype.storm.tuple;

import java.lang.Iterable;

public class Fields //implements Iterable<String>
{
	String tmp;

	public Fields()
	{

	}

	public Fields(String s)  //TODO fix para cumplir: Fields(String... fields)
	{
		tmp = s;
	}

	public String toString()
	{
		return tmp;
	}
}