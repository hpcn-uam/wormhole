package es.hpcn.wormhole;

public class Worm
{
	static
	{
		System.loadLibrary("javaworm");
	}

	static Worm myWorm = null;

	private native int init();

	public Worm() throws Exception
	{
		if (init() != 0) {
			throw new Exception("Failed to initialize JavaWorm library");
		}

		if (myWorm == null) {
			myWorm = this;

		} else {
			throw new Exception("Worm alredy instantiated");
		}
	}

	static public Worm getInstance() throws Exception
	{
		if (myWorm != null) {
			return myWorm;

		} else {
			return new Worm();
		}
	}

	public native int halt();

	public native int recv(byte[] data);
	public native int send(byte[] data);

	public native int recv(String data);
	public native int send(String data);

	public int recv(Object data)
	{
		if (data instanceof String) {
			return recv((String)data);

		} else {
			return 0;
		}
	}
	public int send(Object data)
	{
		if (data instanceof String) {
			return send((String)data);

		} else {
			return 0;
		}
	}

	public native void flushIO();

	public void dispose() //TODO check if ok
	{
		halt();
	}

	/*************/
	public native int getId();
}