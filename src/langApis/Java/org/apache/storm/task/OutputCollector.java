package backtype.storm.task;

import es.hpcn.wormhole.Worm;

import backtype.storm.tuple.Tuple;

import java.util.List;

public class OutputCollector //extends WorkerTopologyContext //implements IMetricsContext
{
	static Worm w = null;
	public OutputCollector() throws Exception
	{
		w = new Worm();
	}

	public void	ack(Tuple input)
	{

	}

	public List<Integer> emit(List<Object> tuple)
	{
		for (Object o : tuple) {
			w.send((String)o); //TODO posible future errors
		}

		return null;
	}
}