////////////////////////////////////////////////////
// InstallShield Java3 Password Cracker           //
// (C)thewd, thewd@hotmail.com                    //
////////////////////////////////////////////////////
// Able to load a class in byte form from a class //
// on the users file system, or within a JAR file //
////////////////////////////////////////////////////
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.IOException;

public class FileClassLoader extends ClassLoader
{
	private byte[] getClassBytes(String classFileName)
	{
		File classFile = new File(classFileName);
		int classFileLength = (int)(classFile.length());
		byte[] fileBuffer = new byte[classFileLength];
		InputStream classFileIS = null;

		try
		{
			classFileIS = new FileInputStream(classFile);
			int bytesReadInTotal = 0;
			int bytesRead;
			while (bytesReadInTotal < classFileLength)
			{
				bytesRead = classFileIS.read(fileBuffer, bytesReadInTotal, classFileLength - bytesReadInTotal);
				if (bytesRead < 0) break;
				bytesReadInTotal += bytesRead;
			}
		}
		catch(FileNotFoundException e)
		{
			return null;
		}
		catch(IOException e)
		{
			return null;
		}

		return fileBuffer;
	}

	public Class loadClassFromFile(String classFileName)
	{
		Class classFile = null;
		InputStream classFileIS = null;
		byte[] fileBuffer = this.getClassBytes(classFileName);

		if (fileBuffer != null)
		{
			classFile = super.defineClass(null, fileBuffer, 0, fileBuffer.length);
			if (classFile != null)
			{
				resolveClass(classFile);
				return classFile;
			}
		}

		return null;
	}

	public Class loadClassFromJar(String jarName, String classFileName)
	{
		Class classFile = null;
		JarResources jarRes = new JarResources(jarName);
		byte[] fileBuffer = jarRes.getResource(classFileName);

		if (fileBuffer != null)
		{
			classFile = super.defineClass(null, fileBuffer, 0, fileBuffer.length);
			if (classFile != null)
			{
				resolveClass(classFile);
				return classFile;
			}
		}

		return null;
	}
}