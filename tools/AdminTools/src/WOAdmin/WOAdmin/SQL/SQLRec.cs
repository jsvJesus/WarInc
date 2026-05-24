using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
    public class SQLInsertBuilder
    {
        public string fields = ""; // xx, yy
        public string values = ""; // @xx, @yy
        public string sets = ""; // set xx=@xx, yy=@yy
        public SqlParameterCollection sqlp;

        public SQLInsertBuilder(SqlParameterCollection in_sqlp)
        {
            sqlp = in_sqlp;
        }

        public void Add(string var_name, object var_data)
        {
            if (fields.Length > 0)
                fields += ", ";
            fields += var_name;

            if (values.Length > 0)
                values += ", ";
            values += "@" + var_name;

            if (sets.Length > 0)
                sets += ", ";
            sets += var_name + "=@" + var_name;

            sqlp.AddWithValue("@" + var_name, var_data);
        }
    };

    public enum SQLDataType
    {
        NONE = 0,
        STRING = 1,
        INT = 2,
        DOUBLE = 3,
        DATETIME = 4,
    }
    
    public class SQLField
    {
        public string name;
        public SQLDataType type2 = SQLDataType.NONE;

        //public Object var;
        public string v_string = "";
        public int VARCHAR_LEN = 512;

        public double v_double = 0.0f;
        public int v_int = 0;
        public DateTime v_time;

        public Object stored_value = null;  // used for update tracking in DBWOAdminChange

        public SQLField(string in_name)
        {
            name = in_name;
        }

        // return if entry was edited
        public bool IsEdited()
        {
            if (stored_value == null)
            {
                MessageBox.Show(string.Format("fld.stored_value is null for {0}", name));
                return false;
            }

            string str1 = stored_value.ToString();
            string str2 = ToObject().ToString();
            if (str1 == str2)
                return false;

            return true;
        }

        public override string ToString()
        {
            switch (type2)
            {
                default:
                    throw new ArgumentException("invalid type2 " + type2);
                
                case SQLDataType.STRING:
                    // remove all string after maximum varchar length
                    if (v_string.Length > VARCHAR_LEN)
                        v_string = v_string.Remove(VARCHAR_LEN);

                    return v_string;

                case SQLDataType.INT:
                    return v_int.ToString();
                
                case SQLDataType.DOUBLE:
                    return v_double.ToString();

                case SQLDataType.DATETIME:
                    return v_time.ToString("s");
            }
        }

        public int AsInt()
        {
            if(type2 != SQLDataType.INT)
                throw new ArgumentException(name + " not a int");
            return v_int;
        }

        public object ToObject()
        {
            switch (type2)
            {
                default:
                    throw new ArgumentException("invalid type2 " + type2);
                
                case SQLDataType.STRING:
                    // remove all string after maximum varchar length
                    if (v_string.Length > VARCHAR_LEN)
                        v_string = v_string.Remove(VARCHAR_LEN);

                    return v_string;

                case SQLDataType.INT:
                    return v_int;
                
                case SQLDataType.DOUBLE:
                    return v_double;

                case SQLDataType.DATETIME:
                    return v_time;
            }
        }

        public bool FromReader(SqlDataReader r)
        {
            try
            {
                stored_value = r[name];
                FromObject(stored_value);
            }
            catch
            {
                string err = String.Format("There is no field {0}", name);
                MessageBox.Show(err, "SQLRec");
                throw;
            }
            return true;
        }

        public bool FromObject(Object o)
        {
            // hack, damnit.
            Type t = o.GetType();
            if(t == typeof(int))
            {
                v_int = Convert.ToInt32(o);
                type2  = SQLDataType.INT;
            }
            else if (t == typeof(string))
            {
                v_string = Convert.ToString(o);
                type2 = SQLDataType.STRING;
            }
            else if (t == typeof(double) || t == typeof(float))
            {
                v_double = Convert.ToDouble(o);
                type2 = SQLDataType.DOUBLE;
            }
            else if (t == typeof(System.DateTime))
            {
                v_time = Convert.ToDateTime(o);
                type2 = SQLDataType.DATETIME;
            }
            else if (t == typeof(DBNull))
            {
                // special case for AccountInfo.admin_note
                v_string = "";
                type2 = SQLDataType.STRING;
            }
            else
            {
                MessageBox.Show(o.GetType().ToString(), "SQLField");
                return false;
            }
            return true;
        }
    }

    public class SQLRec
    {
        public string TABLE_ID = null;
        public SQLField RECORD_ID = null; // variable for unique ID of this record

        public List<SQLField> allFields_ = new List<SQLField>();

        public ListViewItem ui_lvi = null;  // pointer to listview containing this entry

        public SQLRec CloneRecWithoutData()
        {
            SQLRec rec = new SQLRec();

            if(TABLE_ID != null)
                rec.TABLE_ID = TABLE_ID;
            if(RECORD_ID != null)
                rec.RECORD_ID = RECORD_ID;

            foreach(var fld1 in allFields_)
            {
                SQLField fld2 = new SQLField(fld1.name);
                rec.allFields_.Add(fld2);
            }

            return rec;
        }

        protected void RegisterField(SQLField fld)
        {
            if (GetField(fld.name) != null)
            {
                MessageBox.Show("SQLField " + fld.name + " already registered");
                System.Diagnostics.Debug.Assert(false);
                return;
            }
            allFields_.Add(fld);
            return;
        }

        public SQLField GetField(string name)
        {
            foreach (SQLField fld in allFields_)
            {
                if (fld.name == name)
                    return fld;
            }
            return null;
        }

        public bool Fill(SqlDataReader r)
        {
            if (RECORD_ID != null)
                RECORD_ID.FromReader(r);

            foreach (SQLField fld in allFields_)
            {
                // parse that object.
                fld.FromReader(r);
            }

            return true;
        }

        public void BuildWhereString(ref string where)
        {
            if (RECORD_ID.type2 == SQLDataType.NONE)
                throw new ArgumentException("RECORD_ID must not be null in update");

            where = string.Format("{0}={1}", RECORD_ID.name, RECORD_ID.ToString());
        }

        public void BuildInsertString(ref string fields, ref string values, SqlParameterCollection sqlp)
        {
            if (RECORD_ID != null && RECORD_ID.type2 != SQLDataType.NONE)
                throw new ArgumentException("RECORD_ID must be null in insert");

            SQLInsertBuilder bld = new SQLInsertBuilder(sqlp);
            foreach (SQLField fld in allFields_)
            {
                bld.Add(fld.name, fld.ToObject());
            }

            fields = bld.fields;
            values = bld.values;
        }

        public void BuildUpdateString(ref string sets, SqlParameterCollection sqlp)
        {
            if (RECORD_ID.type2 == SQLDataType.NONE)
                throw new ArgumentException("RECORD_ID must not be null in update");

            SQLInsertBuilder bld = new SQLInsertBuilder(sqlp);
            foreach (SQLField fld in allFields_)
            {
                if (!fld.IsEdited())
                    continue;

                bld.Add(fld.name, fld.ToObject());
            }

            sets = bld.sets;
        }
    }
}
