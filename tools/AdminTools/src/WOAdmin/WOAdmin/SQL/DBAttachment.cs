using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
    public class DBAttachment : DBCommonItem
    {
        public static bool IS_THIS_CATEGORY(int category)
        {
            return category == 19;
        }

        public static EditField_Combo.SPairs CategoryValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("19", "FPS Attachment"), 
        };

        public static EditField_Combo.SPairs TypeValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("0", "Muzzle"), 
            new EditField_Combo.SPair("1", "Upper Rail"), 
            new EditField_Combo.SPair("2", "Left Rail"), 
            new EditField_Combo.SPair("3", "Bottom Rail"), 
            new EditField_Combo.SPair("4", "Clip"), 
            new EditField_Combo.SPair("5", "Receiver"), 
            new EditField_Combo.SPair("6", "Stock"), 
            new EditField_Combo.SPair("7", "Barrel"), 
            new EditField_Combo.SPair("8", "Paint"), 
        };

        public SQLField Type = new SQLField("Type");
        public SQLField SpecID = new SQLField("SpecID");
        public SQLField Damage = new SQLField("Damage");
        public SQLField Range = new SQLField("Range");
        public SQLField Firerate = new SQLField("Firerate");
        public SQLField Recoil = new SQLField("Recoil");
        public SQLField Spread = new SQLField("Spread");
        public SQLField Clipsize = new SQLField("Clipsize");
        public SQLField Scopemag = new SQLField("Scopemag");
        public SQLField Scopetype = new SQLField("Scopetype");
        public SQLField AnimPrefix = new SQLField("AnimPrefix");
        public SQLField MuzzleParticle = new SQLField("MuzzleParticle");
        public SQLField FireSound = new SQLField("FireSound");

        public static string GET_TABLE_NAME()
        {
            return "Items_Attachments";
        }

        public DBAttachment()
        {
            TABLE_ID = GET_TABLE_NAME();

            RegisterField(Type);
            RegisterField(SpecID);
            RegisterField(Damage);
            RegisterField(Range);
            RegisterField(Firerate);
            RegisterField(Recoil);
            RegisterField(Spread);
            RegisterField(Clipsize);
            RegisterField(Scopemag);
            RegisterField(Scopetype);
            RegisterField(AnimPrefix);
            RegisterField(MuzzleParticle);
            RegisterField(FireSound);
        }

        public void SetNewRecordDefaults()
        {
            SetNewItemDefaults(19);

            Type.FromObject(0);
            SpecID.FromObject(0);
            Damage.FromObject(0);
            Range.FromObject(0);
            Firerate.FromObject(0);
            Recoil.FromObject(0);
            Spread.FromObject(0);
            Clipsize.FromObject(0);
            Scopemag.FromObject("");
            Scopetype.FromObject("");
            AnimPrefix.FromObject("");
            MuzzleParticle.FromObject("");
            FireSound.FromObject("");
        }
    }
}
