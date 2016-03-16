package es.hpcn.wormhole;

public class Einstein
{
	static
	{
		System.loadLibrary("javaworm");
	}

	private native int init(String configFileName, String listenIp, int listenPort, boolean autoDeployWorms) throws Exception;

	public Einstein(String configFileName, String listenIp, int listenPort, boolean autoDeployWorms) throws Exception
	{
		if (init(configFileName, listenIp, listenPort, autoDeployWorms) != 0) {
			throw new Exception("Failed to initialize JavaWorm-Einstein library");
		}
	}

}