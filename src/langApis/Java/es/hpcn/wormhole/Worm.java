package es.hpcn.wormhole;

public class Worm
{
	static
	{
		System.loadLibrary("javaworm");
	}

	private native int init();

	public Worm() throws Exception
	{
		if (init() != 0) {
			throw new Exception("Failed to initialize JavaWorm library");
		}
	}

	public native int halt();

	public native int recv(byte[] data);
	public native int send(byte[] data);

	public native int recv(String data);
	public native int send(String data);

	public native void flushIO();

	public void dispose() //TODO check if ok
	{
		halt();
	}

	/*************/
	public native int getId();
}