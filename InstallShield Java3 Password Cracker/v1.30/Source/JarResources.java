///////////////////////////////////////////
// InstallShield Java3 Password Cracker  //
// (C)thewd, thewd@hotmail.com           //
///////////////////////////////////////////
///////////////////////////////////////////
// Uses to retrieve resources/classes    //
// from within a Jar archive             //
///////////////////////////////////////////
import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipInputStream;

public final class JarResources
{
	private Hashtable htSizes = new Hashtable();
	private Hashtable htJarContents = new Hashtable();
	private String jarFileName;

	public JarResources(String jarFileName)
	{
		this.jarFileName = jarFileName;
		this.initialise();
	}

	public byte[] getResource(String name)
	{
		return (byte[])htJarContents.get(name);
	}

	private void initialise()
	{
		try
		{
			ZipFile zipFile = new ZipFile(jarFileName);
			Enumeration zipEnum = zipFile.entries();

			while (zipEnum.hasMoreElements())
			{
				ZipEntry zipEntry = (ZipEntry)zipEnum.nextElement();
				htSizes.put(zipEntry.getName(), new Integer((int)zipEntry.getSize()));
			}

			zipFile.close();

			FileInputStream fis = new FileInputStream(jarFileName);
			BufferedInputStream bis = new BufferedInputStream(fis);
			ZipInputStream zis = new ZipInputStream(bis);
			ZipEntry zipEntry = null;

			while ((zipEntry = zis.getNextEntry()) != null)
			{
				if (zipEntry.isDirectory()) continue;

				int size = (int)zipEntry.getSize();
				if (size == -1) size=((Integer)htSizes.get(zipEntry.getName())).intValue();

				byte[] b = new byte[(int)size];
				int rb = 0;
				int chunk = 0;

				while (((int)size - rb) > 0)
				{
					chunk = zis.read(b, rb, (int)size - rb);
					if (chunk == -1) break;
					rb += chunk;
				}

				htJarContents.put(zipEntry.getName(), b);
			}
		}
		catch (Exception e)
		{
			System.out.println("\nWarning: Unable to open or read the JAR archive");
			System.exit(1);
		}
	}
}